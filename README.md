# DirectDraw Loader
DirectDraw Loader (or DDrawLoader for short) is a simple, dedicated DirectDraw wrapper that allows custom render logic to be injected into the target application in a modular way, without having to modify the target application's source code. It takes care of hooking DirectDraw in the application of choice and provides a plugin interface, through which it loads and injects custom render logic, provided by the user, in the form of a DLL module.

## Plugins
Plugins are user-provided DLL files that expose a simple `RenderPluginAPI` interface. They are dynamically loaded and can draw on the application's surface or respond to its update loop. All render plugins must export a single function:
`__declspec(dllexport) RenderPluginAPI* GetRenderPlugin();`.
This function returns a pointer to a `RenderPluginAPI` structure containing the user's implementation of supplied callback functions:
- `void Init()` [optional] — sets the plugin up before proper use (e.g. load a cursor);
- `void Draw(HDC hdc)` [required] — renders custom graphics on the device;
- `void Update()` [optional] — polled every 16ms (~1 frame if 60 FPS) in the main program loop, can be used to listen for events, such as key presses;
- `bool ExitRequested()` [optional] — returns the exit request state based on a user-defined condition (e.g. a hotkey press);
- `void Shutdown()` [optional] — performs a cleanup if necessary.
The interface is defined in the `render_interface.h` header file.
