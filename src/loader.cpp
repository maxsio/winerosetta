#include <windows.h>
#include <iostream>
#include <filesystem>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char* exePath = ".\\wow.exe";
    std::filesystem::path dllPath = std::filesystem::absolute(".\\winerosetta.dll");
    std::cout << "Absolute DLL Path: " << dllPath << std::endl;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(exePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        return 1;
    }

    void* remoteDllPath = VirtualAllocEx(pi.hProcess, NULL, dllPath.string().length() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (remoteDllPath == NULL) {
        return 1;
    }

    if (!WriteProcessMemory(pi.hProcess, remoteDllPath, dllPath.string().c_str(), dllPath.string().length() + 1, NULL)) {
        return 1;
    }

    // Use NtCreateThreadEx for better performance
    HANDLE hThread = nullptr;
    NTSTATUS status = NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, nullptr, pi.hProcess, (LPTHREAD_START_ROUTINE)LoadLibraryA, remoteDllPath, FALSE, 0, 0, 0, nullptr);
    if (!NT_SUCCESS(status)) {
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(pi.hProcess, remoteDllPath, 0, MEM_RELEASE);
    CloseHandle(hThread);

    ResumeThread(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}