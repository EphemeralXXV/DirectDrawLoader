#pragma once

#include <ddraw.h>

#include "render_interface.h"

namespace hooks {
    void Setup();
    void SignalExit() noexcept;
    bool ShouldExit() noexcept;
    void Destroy() noexcept;

    bool LoadRenderPlugin(const char* dllPath) noexcept;
    RenderPluginAPI* GetPluginAPI() noexcept;

    HRESULT WINAPI HookedFlip(LPDIRECTDRAWSURFACE surface, LPDIRECTDRAWSURFACE target, DWORD flags) noexcept;
    HRESULT WINAPI HookedCreateSurface(LPDIRECTDRAW dd, LPDDSURFACEDESC desc, LPDIRECTDRAWSURFACE* surface, IUnknown* unknown) noexcept;
    HRESULT WINAPI HookedDirectDrawCreate(GUID* lpGuid, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter) noexcept;
}