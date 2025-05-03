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

## Installation guide
1. Download the latest release of DirectDraw Loader.
2. Unpack the archive anywhere on your disk.
3. Put your DLL plugin in `/DDrawLoader/plugins/`.
4. Open the `config.ini` file in the root DirectDraw Loader directory and:
   4.1. Go to the `Target` section and change the value of `exePath` to the path of your target application (can be absolute or relative) — for example `C:/Program Files/SampleDDrawApp/app.exe`.
   4.2. Go to the `Plugin` section and change the value of `dllName` to the name of your plugin (including the `.dll` extension) — for example, `plugin.dll`.
6. Run `DDrawLoader.exe` as an administrator.
Note: the executable may be flagged as malware by your antivirus software, so make sure to add it as an exception. Considering the program involves creating processes, injecting DLLs and intercepting renderer calls, it is something to be expected. If you are hesitant to use the executable, you may use a DLL injector, such as Xenos, to inject the DLLs yourself; the executable simply automates this process.

## Tested applications
The idea for this program came from attempting to create a mod menu in the 1999 video game Mobil 1 Rally Championship which relies heavily on DirectDraw. As such, it is the one appliaction guaranteed to be supported. Tests on a few other video games have also been performed. Below is a brief rundown:
| Game                               | Year | Works?                            | Description                                                                                                           | Notes                                          |
|------------------------------------|------|-----------------------------------|-----------------------------------------------------------------------------------------------------------------------|------------------------------------------------|
| Mobil 1 Rally Championship         | 1999 | ✔️ Yes                             | Rendering works both in game and in the menu.                                                                         |                                                |
| Sega Rally Championship            | 1997 | ✔️ Yes                             | Rendering works both in game and in the menu.                                                                         |                                                |
| Worms World Party                  | 2001 | ✔️ Yes                             | Rendering works both in game and in the menu.                                                                         | Original 2001 release                          |
| Grand Theft Auto                   | 1997 | ✔️/⚠️ Partially                     | Rendering works, but only in game (not in the menu).                                                                  | Windows version                                |
| 1nsane                             | 2000 | ❗ No, but might be patched        | Hook incomplete (`Flip` is never called). Possibly an easy fix, as it's a late-stage call.                            |                                                |
| Brave Dwarves 2                    | 2002 | ❗❗ No, but might be patched       | Hook incomplete (`CreateSurface` is never called). Possibly a moderately challenging fix, as it's a mid-stage call.   | Tested on versions 1.02, 1.08, Gold and Deluxe |
| Demolition Champions               | 2003 | ❗❗ No, but might be patched       | Hook incomplete (`CreateSurface` is never called). Possibly a moderately challenging fix, as it's a mid-stage call.   | Also called *Smash Up Derby*                   |
| Ignition                           | 1997 | ❗❗❗ No, but might be patched      | Hook incomplete (`DirectDrawCreate` is never called). Possibly a highly challenging fix, as it's an early-stage call. | 3DFx patch version                             |
| RollerCoaster Tycoon 2             | 2002 | ❗❗❗ No, but might be patched      | Hook incomplete (`DirectDrawCreate` is never called). Possibly a highly challenging fix, as it's an early-stage call. |                                                |
| Tzar: The Burden of the Crown      | 2000 | ❗❗❗ No, but might be patched      | Hook incomplete (`DirectDrawCreate` is never called). Possibly a highly challenging fix, as it's an early-stage call. |                                                |
| Need for Speed II: Special Edition | 1997 | ❌ No, and likely won't be patched | Cannot hook (fails to find `ddraw.dll`)                                                                               |                                                |
