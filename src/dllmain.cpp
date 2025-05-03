#include <windows.h>
#include <exception>

#include "hooks.h"
#include "render_interface.h"
#include "config.h"

DWORD WINAPI MainThread(LPVOID lpReserved) {
    try {
        // Step 1: Install hooks
        hooks::Setup();

        // Step 2: Load plugin
        auto hModule = static_cast<HMODULE>(lpReserved);
        std::string pluginPath = config::GetPluginPath(hModule);
        if(!hooks::LoadRenderPlugin(pluginPath.c_str())) { 
            MessageBoxA(0, "Failed to load render plugin!", "Plugin Load Error", MB_ICONERROR);
            return 1;
        }
    }
    catch(const std::exception& error) {
        MessageBeep(MB_ICONERROR);
        MessageBox(
            0,
            error.what(),
            "ERROR",
            MB_OK | MB_ICONEXCLAMATION
        );
    }

    /* Main program loop */
    while(true) {
        RenderPluginAPI* pluginAPI = hooks::GetPluginAPI();

        if(pluginAPI->Update) {
            pluginAPI->Update();
        }

        bool exitRequested = pluginAPI->ExitRequested && pluginAPI->ExitRequested();
        bool shouldExit = hooks::ShouldExit();
        if(exitRequested || shouldExit) {
            break;
        }
        Sleep(16); // ~60fps
    }

    // Clean up the hooks, unload the DLL and exit safely
    hooks::Destroy();
    FreeLibraryAndExitThread(static_cast<HMODULE>(lpReserved), 0);
}

// DLL Entry point
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    if(dwReason == DLL_PROCESS_ATTACH) {

        // Disable DLL notifications
        DisableThreadLibraryCalls(hModule);

        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    else if(dwReason == DLL_PROCESS_DETACH) {
        hooks::SignalExit();
    }
    return TRUE;
}