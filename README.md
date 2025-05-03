# DirectDraw Loader
***DirectDraw Loader*** (or *DDrawLoader* for short) is a simple, dedicated DirectDraw wrapper that allows custom render logic to be injected into the target application in a modular way, without having to modify the target application's source code. It takes care of hooking DirectDraw in the application of choice and provides a plugin interface, through which it loads and injects custom render logic, provided by the user, in the form of a DLL module.

## Plugins
**Plugins** are user-provided DLL files that expose a minimal `RenderPluginAPI` interface. These are dynamically loaded to overlay custom graphics or perform per-frame logic. All render plugins must export a single function:

`__declspec(dllexport) RenderPluginAPI* GetRenderPlugin()`

which returns a pointer to a `RenderPluginAPI` structure containing the user's implementation of supplied callback functions:
- `void Init()` [optional] — Called once after plugin load; use for setup (e.g., loading assets).
- `void Draw(HDC hdc)` [required] — Draws custom graphics to the provided device context (called each frame).
- `void Update()` [optional] — Polled at ~60 Hz; useful for handling input or time-based updates.
- `bool ExitRequested()` [optional] — Returns the exit request state based on a user-defined condition (e.g. a hotkey press);
- `void Shutdown()` [optional] — Called during DLL unload; use for cleanup.

The interface is defined in `render_interface.h`. Naturally, in order to compile a plugin, you must include this header file.

## Installation guide
1. Download the latest release of *DirectDraw Loader*.
2. Unpack the archive anywhere on your disk.
3. Place your DLL plugin in `/DDrawLoader/plugins/`.
4. Open the `config.ini` file in the *DirectDraw Loader* root folder:
   4.1. Under `[Target]`, set `exePath` to the path of your target application (can be absolute or relative) — for example `C:/Program Files/SampleDDrawApp/app.exe`.
   4.2. Under `[Plugin]`, set `dllName` to the name of your plugin (including the `.dll` extension) — for example, `plugin.dll`.
6. Run `DDrawLoader.exe` as an administrator.
   
**Note:** Antivirus software may flag the executable due to its behavior (DLL injection, hooking, etc.), so make sure to add it as an exception. If preferred, you can use a third-party injector (e.g., Xenos) and inject the DLL manually; the executable simply automates this process.

## Tested applications
The idea for this program came from attempting to create a mod menu in the 1999 video game Mobil 1 Rally Championship which relies heavily on DirectDraw. As such, it is the one application guaranteed to be supported. Tests on a few other video games have also been performed. Below is a brief rundown.
| Game                                 | Year | Works?                             | Description                                                                                                           | Notes                                          |
|--------------------------------------|------|------------------------------------|-----------------------------------------------------------------------------------------------------------------------|------------------------------------------------|
| *Mobil 1 Rally Championship*         | 1999 | ✔️ Yes                             | Rendering works both in game and in the menu.                                                                         |                                                |
| *Sega Rally Championship*            | 1997 | ✔️ Yes                             | Rendering works both in game and in the menu.                                                                         |                                                |
| *Worms World Party*                  | 2001 | ✔️ Yes                             | Rendering works both in game and in the menu.                                                                         | Original 2001 release                          |
| *Grand Theft Auto*                   | 1997 | ✔️/⚠️ Partially                   | Rendering works, but only in game (not in the menu).                                                                  | Windows version                                |
| *1nsane*                             | 2000 | ❗ No, but might be patched         | Hook incomplete (`Flip` is never called). Possibly an easy fix, as it's a late-stage call.                            |                                                |
| *Brave Dwarves 2*                    | 2002 | ❗❗ No, but might be patched        | Hook incomplete (`CreateSurface` is never called). Possibly a moderately challenging fix, as it's a mid-stage call.   | Tested on versions 1.02, 1.08, Gold and Deluxe |
| *Demolition Champions*               | 2003 | ❗❗ No, but might be patched        | Hook incomplete (`CreateSurface` is never called). Possibly a moderately challenging fix, as it's a mid-stage call.   | Also called *Smash Up Derby*                   |
| *Ignition*                           | 1997 | ❗❗❗ No, but might be patched       | Hook incomplete (`DirectDrawCreate` is never called). Possibly a highly challenging fix, as it's an early-stage call. | 3DFx patch version                             |
| *RollerCoaster Tycoon 2*             | 2002 | ❗❗❗ No, but might be patched       | Hook incomplete (`DirectDrawCreate` is never called). Possibly a highly challenging fix, as it's an early-stage call. |                                                |
| *Tzar: The Burden of the Crown*      | 2000 | ❗❗❗ No, but might be patched       | Hook incomplete (`DirectDrawCreate` is never called). Possibly a highly challenging fix, as it's an early-stage call. |                                                |
| *Need for Speed II: Special Edition* | 1997 | ❌ No, and likely won't be patched | Cannot hook (fails to find `ddraw.dll`)                                                                               |                                                |

All tests carried out on Windows 10 Pro x64.

## Limitations
- Due to its very nature, *DirectDraw Loader* is limited to 32-bit (x86) environments. This includes the loader itself, user plugins, and the target applications — 64-bit processes are not supported.
- Supported DirectDraw versions are 1 through 6. Version 7 is not yet supported, but is planned for a future release.
- Currently, only *page flipping* (`Flip`) is hooked. Support for *blitting* (e.g., `Blt`, `BltFast`) is under consideration.
- Applications running in windowed mode or using wrappers such as *DDrawCompat*, *dgVoodoo*, or *DxWnd* are likely incompatible. Some exceptions exist, but broad support is not guaranteed at this stage.
Planned improvements may address some of these limitations over time, depending on interest and feasibility.

## Acknowledgements
- *DirectDraw Loader* relies on the [*MinHook*](https://github.com/TsudaKageyu/minhook) library by TsudaKageyu for API hooking.
- Portions of the source code and project structure based on the following YouTube videos:
  - [*INTERNAL IMGUI MENU (DIRECTX 9)*](https://www.youtube.com/watch?v=vF5fzIDUJVw) by cazz.
  - [*External Overlay in C++ | Works For ANY Game*](https://www.youtube.com/watch?v=BIZyxja3Qls) by CasualGamer.
