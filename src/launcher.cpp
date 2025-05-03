#include <windows.h>
#include <shlwapi.h>
#include <string>
#include <iostream>
#include <fstream>

#include "config.h"

bool InjectDLL(const std::string& dllPath, const std::string& exePath) {
    // Step 1: Launch the target app process
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    
    // Specify the process creation flags
    DWORD dwCreationFlags = CREATE_SUSPENDED;

    // Specify the target app directory (otherwise it might fail to locate its files)
    char exeDir[MAX_PATH];
    strcpy_s(exeDir, exePath.c_str());
    PathRemoveFileSpecA(exeDir); 

    if(!CreateProcessA(exePath.c_str(), nullptr, nullptr, nullptr, FALSE, dwCreationFlags, nullptr, exeDir, &si, &pi)) {
        throw std::runtime_error("Could not start the target app process.");
    }

    // Step 2: Get the address of LoadLibraryA (from kernel32.dll)
    LPVOID pLoadLibrary = reinterpret_cast<void*>(GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));
    if(pLoadLibrary == nullptr) {
        TerminateProcess(pi.hProcess, 0);
        throw std::runtime_error("Could not get LoadLibraryA address.");
    }

    // Step 3: Allocate memory in the app process for the DLL path
    LPVOID pDllPath = VirtualAllocEx(pi.hProcess, nullptr, dllPath.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(pDllPath == nullptr) {
        TerminateProcess(pi.hProcess, 0);
        throw std::runtime_error("Could not allocate memory in the target process.");
    }

    // Step 4: Write the DLL path to the allocated memory
    if(!WriteProcessMemory(pi.hProcess, pDllPath, dllPath.c_str(), dllPath.length() + 1, nullptr)) {
        TerminateProcess(pi.hProcess, 0);
        throw std::runtime_error("Could not write DLL path to the target process.");
    }

    // Step 5: Create a remote thread to load the DLL
    HANDLE hThread = CreateRemoteThread(pi.hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPath, 0, nullptr);
    if(hThread == nullptr) {
        TerminateProcess(pi.hProcess, 0);
        throw std::runtime_error("Could not create remote thread.");
    }

    // Step 6: Wait for the thread to finish (optional)
    WaitForSingleObject(hThread, INFINITE);

    // Step 7: Clean up
    CloseHandle(hThread);
    VirtualFreeEx(pi.hProcess, pDllPath, 0, MEM_RELEASE);
    ResumeThread(pi.hThread);

    // Step 8: Close the process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

int main() {
    HMODULE selfModule = GetModuleHandleA(nullptr);

    std::string baseDir = config::GetModuleDirectory(selfModule);
    std::string configPath = config::GetConfigPath(selfModule);
    std::string ddrawloaderPath = baseDir + "\\ddrawloader.dll";
    std::string exePathAbsolute;

    char exePath[MAX_PATH] = "app.exe";  // Default fallback
    GetPrivateProfileStringA("Target", "exePath", exePath, exePath, MAX_PATH, configPath.c_str());

    if(PathIsRelativeA(exePath)) {
        exePathAbsolute = baseDir + "\\" + exePath;
    }
    else {
        exePathAbsolute = exePath;
    }

    OutputDebugStringA(("[*] DDrawLoader path: " +  ddrawloaderPath + "\n").c_str());
    OutputDebugStringA(("[*] Target EXE path: " + exePathAbsolute + "\n").c_str());

    if(InjectDLL(ddrawloaderPath, exePathAbsolute)) {
        OutputDebugStringA("[*] DDrawLoader injected successfully!\n");
    }
    else {
        throw std::runtime_error("DDrawLoader injection failed.");
    }

    return 0;
}