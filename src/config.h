#pragma once

#include <string>
#include <windows.h>

namespace config {
    std::string GetModuleDirectory(HMODULE module);
    std::string GetConfigPath(HMODULE module);
    std::string GetPluginPath(HMODULE module);
}