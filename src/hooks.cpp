#include <windows.h>
#include <unordered_set>
#include <stdexcept>
#include <atomic>

#include <ddraw.h>

#include "minhook.h"

#include "render_interface.h"
#include "hooks.h"

// VTable indices for Flip and CreateSurface functions
#define VTBL_INDEX_FLIP 11
#define VTBL_INDEX_CREATESURFACE 6

// Typedefs
typedef HRESULT(WINAPI* DirectDrawCreateFn)(GUID*, LPDIRECTDRAW*, IUnknown*);
typedef HRESULT(WINAPI* CreateSurfaceFn)(LPDIRECTDRAW, LPDDSURFACEDESC, LPDIRECTDRAWSURFACE*, IUnknown*);
typedef HRESULT(WINAPI* FlipFn)(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE, DWORD);

// Original functions
DirectDrawCreateFn originalDirectDrawCreate = nullptr;
CreateSurfaceFn originalCreateSurface = nullptr;
FlipFn originalFlip = nullptr;

// Original DDraw surface
LPDIRECTDRAWSURFACE originalSurface = nullptr;

// Set containing hooked VTables used to prevent repeated re-hooking
std::unordered_set<void*> hookedVTables;

// Forced exit flag
static std::atomic<bool> shouldExit = false;

// Shutdown flag to avoid interruptions
static std::atomic<bool> shuttingDown = false;

// DLL plugin to load and its logic
static HMODULE pluginModule = nullptr;
static RenderPluginAPI* pluginAPI = nullptr;

// Load user-provided DLL plugin with custom render logic 
// Called by dllmain to ensure separation of concerns
bool hooks::LoadRenderPlugin(const char* dllPath) noexcept {
    pluginModule = LoadLibraryA(dllPath);
    if(!pluginModule) {
        OutputDebugStringA("[-] Failed to load plugin DLL!\n");
        return false;
    }

    auto getAPI = (RenderPluginAPI*(*)())GetProcAddress(pluginModule, "GetRenderPlugin");
    if(!getAPI) {
        OutputDebugStringA("[-] Failed to find GetRenderPlugin() in plugin DLL!\n");
        return false;
    }

    pluginAPI = getAPI();

    // Validate required fields
    if (!pluginAPI || !pluginAPI->Draw || !pluginAPI->ExitRequested) {
        OutputDebugStringA("[-] Plugin API missing required fields!\n");
        return false;
    }

    // Optional - prepare the plugin for drawing
    if(pluginAPI->Init) {
        pluginAPI->Init();
    }

    return true;
}

// Plugin API getter
RenderPluginAPI* hooks::GetPluginAPI() {
    return pluginAPI;
}

// Hooked (custom) Flip function
HRESULT WINAPI hooks::HookedFlip(LPDIRECTDRAWSURFACE surface, LPDIRECTDRAWSURFACE target, DWORD flags) noexcept {

    // Skip during shutdown
    if(shuttingDown) {
        return originalFlip(surface, target, flags);
    }

    DDSCAPS caps = { 0 };
    caps.dwCaps = DDSCAPS_BACKBUFFER;
    LPDIRECTDRAWSURFACE pBackBuffer = nullptr;

    if(!SUCCEEDED(surface->GetAttachedSurface(&caps, &pBackBuffer))) {
        OutputDebugStringA("[-] Failed to get attached surface!\n");
        return DDERR_GENERIC;
    }
    OutputDebugStringA("[*] Got attached surface!\n");

    HDC hdc;
    if(!SUCCEEDED(pBackBuffer->GetDC(&hdc))) {
        OutputDebugStringA("[-] Failed to get device context for back buffer!\n");
        return DDERR_GENERIC;
    }

    // Run the plugin-provided render logic
    if(pluginAPI && pluginAPI->Draw) {
        pluginAPI->Draw(hdc);
    }
    else {
        OutputDebugStringA("[-] Plugin API missing or not implemented correctly!!\n");
        return DDERR_GENERIC;
    }

    pBackBuffer->ReleaseDC(hdc);
    OutputDebugStringA("[*] Device context released!\n");

    pBackBuffer->Release();
    OutputDebugStringA("[*] Back buffer released!\n");

    // Return control to the target app
    return originalFlip(surface, target, flags);
}

// Hooked (custom) CreateSurface function
HRESULT WINAPI hooks::HookedCreateSurface(LPDIRECTDRAW dd, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE* surface, IUnknown* unknown) noexcept {
    OutputDebugStringA("[*] Hooked CreateSurface!\n");

    HRESULT hr = originalCreateSurface(dd, desc, surface, unknown);
    if(!(SUCCEEDED(hr) && surface && *surface)) {
        OutputDebugStringA("[-] Failed to create surface!\n");
        return DDERR_GENERIC;
    }
    void** vtable = *reinterpret_cast<void***>(*surface);

    // Hook Flip (index 11 in IDirectDraw vtable)
    if(hookedVTables.find(vtable) == hookedVTables.end()) {
        if(MH_CreateHook(
            vtable[VTBL_INDEX_FLIP],
            reinterpret_cast<void*>(&HookedFlip),
            reinterpret_cast<void**>(&originalFlip)
        ) != MH_OK) {
            OutputDebugStringA("[!] Failed to hook Flip!\n");
        }
        if(MH_EnableHook(vtable[VTBL_INDEX_FLIP]) != MH_OK) {
            OutputDebugStringA("[!] Failed to enable Flip hook!\n");
        }

        hookedVTables.insert(vtable);
        OutputDebugStringA("[*] Hooked Flip from surface!\n");
    }
    else {
        OutputDebugStringA("[~] Flip already hooked for this surface.\n");
    }
    return hr;
}


// Hooked (custom) DirectDrawCreate function
HRESULT WINAPI hooks::HookedDirectDrawCreate(GUID* lpGuid, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter) noexcept {
    OutputDebugStringA("[*] Hooked DirectDrawCreate!\n");

    HRESULT hr = originalDirectDrawCreate(lpGuid, lplpDD, pUnkOuter);
    if(!(SUCCEEDED(hr) && lplpDD && *lplpDD)) {
        OutputDebugStringA("[-] Failed to inititalize DirectDraw object!\n");
        return DDERR_GENERIC;
    }
    void** vtable = *reinterpret_cast<void***>(*lplpDD);

    // Hook CreateSurface (index 6 in IDirectDraw vtable)
    if(MH_CreateHook(
        vtable[VTBL_INDEX_CREATESURFACE],
        reinterpret_cast<void*>(&HookedCreateSurface),
        reinterpret_cast<void**>(&originalCreateSurface)
    ) != MH_OK) {
        OutputDebugStringA("[!] Failed to hook CreateSurface!\n");
    }
    if(MH_EnableHook(vtable[VTBL_INDEX_CREATESURFACE]) != MH_OK) {
        OutputDebugStringA("[!] Failed to enable Flip hook!\n");
    }

    OutputDebugStringA("[*] Hooked CreateSurface from device!\n");
    return hr;
}

// Setup all hooks
void hooks::Setup() {
    MH_Initialize();

    // Poll for ddraw.dll for 1 second (some apps might not load it straight away)
    HMODULE ddraw = nullptr;
    int attempts = 0;
    while((ddraw = GetModuleHandleA("ddraw.dll")) == nullptr && attempts++ < 10) {
        OutputDebugStringA("[*] Failed to find ddraw.dll! Retrying...\n");
        Sleep(100);  // Wait 100 ms
    }
    if(!ddraw) {
        throw std::runtime_error("Failed to find ddraw.dll");
    }

    auto addr = GetProcAddress(ddraw, "DirectDrawCreate");
    if(!addr) {
        throw std::runtime_error("Failed to find DirectDrawCreate");
    }

    if(MH_CreateHook(
        reinterpret_cast<void*>(addr),
        reinterpret_cast<void*>(&HookedDirectDrawCreate),
        reinterpret_cast<void**>(&originalDirectDrawCreate)
    ) != MH_OK) {
        throw std::runtime_error("Failed to create hook!");
    }

    if(MH_EnableHook(reinterpret_cast<void*>(addr)) != MH_OK) {
        throw std::runtime_error("Failed to enable hook!");
    }

    OutputDebugStringA("[*] All hooks installed!\n");
}

// Forced exit state setter and getter
void hooks::SignalExit() noexcept {
    shouldExit = true;
}
bool hooks::ShouldExit() noexcept {
    return shouldExit;
}

// Unhook cleanly
void hooks::Destroy() noexcept {
    shuttingDown = true;

    // Let any in-flight hook calls return before tearing down
    Sleep(100);
    
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if(pluginAPI && pluginAPI->Shutdown) {
        pluginAPI->Shutdown();
    }
    if(pluginModule) {
        FreeLibrary(pluginModule);
    }
}