#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Required plugin fields: must not be nullptr
    // Draw, ExitRequested

    // Optional: Init, Update, Shutdown

    typedef void (*PluginInitFn)();
    typedef void (*PluginDrawFn)(HDC hdc);
    typedef void (*PluginUpdateFn)();
    typedef bool (*PluginExitRequestedFn)();
    typedef void (*PluginShutdownFn)();

    struct RenderPluginAPI {
        PluginInitFn Init;
        PluginDrawFn Draw;
        PluginUpdateFn Update;
        PluginExitRequestedFn ExitRequested;
        PluginShutdownFn Shutdown;
    };

    // Exported function that plugins must implement
    __declspec(dllexport) RenderPluginAPI* GetRenderPlugin();

#ifdef __cplusplus
}
#endif