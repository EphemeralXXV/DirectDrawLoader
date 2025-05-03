#include <windows.h>
#include <filesystem>

#include "config.h"

namespace fs = std::filesystem;

// Returns the directory path of the current module (EXE or DLL).
std::string config::GetModuleDirectory(HMODULE module) {
    char path[MAX_PATH] = {};
    GetModuleFileNameA(module, path, MAX_PATH);
    return fs::path(path).parent_path().string();
}

// Returns the path to config.ini (same dir as the module).
std::string config::GetConfigPath(HMODULE module) {
    return GetModuleDirectory(module) + "\\config.ini";
}

// Returns the path to the plugin DLL, based on config.ini or default.
std::string config::GetPluginPath(HMODULE module) {
    std::string iniPath = GetConfigPath(module);
    char pluginName[MAX_PATH] = "render.dll"; // default fallback

    GetPrivateProfileStringA(
        "Plugin",
        "dllName",
        "render.dll",
        pluginName,
        MAX_PATH,
        iniPath.c_str()
    );

    return GetModuleDirectory(module) + "\\plugins\\" + std::string(pluginName);
}