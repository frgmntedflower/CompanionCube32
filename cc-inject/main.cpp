#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <thread>

using namespace std;

#define INJECTOR_FAIL(s) { \
    printf(s " Error code: %d\n", GetLastError()); \
    if (hProc) CloseHandle(hProc); \
    return 1; \
}

DWORD get_proc_by_name(const char* procName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    do {
        if (strcmp(pe.szExeFile, procName) == 0) {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe));

    CloseHandle(hSnapshot);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: cc_injector.exe <name_of_process> <path_to_dll>\n";
        return 1;
    }

    const char* procName = argv[1];
    const char* dllPath = argv[2];

    DWORD procId = get_proc_by_name(procName);
    if (procId == 0) {
        cout << "Process not found\n";
        return 1;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (!hProc) INJECTOR_FAIL("OpenProcess failed");

    LPVOID lpBaseOfDll = VirtualAllocEx(
        hProc,
        nullptr,
        strlen(dllPath) + 1,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    if (!lpBaseOfDll) INJECTOR_FAIL("VirtualAllocEx failed");

    if (!WriteProcessMemory(hProc, lpBaseOfDll, dllPath, strlen(dllPath) + 1, nullptr))
        INJECTOR_FAIL("WriteProcessMemory failed");

    HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!kernel32) INJECTOR_FAIL("GetModuleHandle failed");

    FARPROC loadLibA = GetProcAddress(kernel32, "LoadLibraryA");
    if (!loadLibA) INJECTOR_FAIL("GetProcAddress failed");

    HANDLE hThread = CreateRemoteThread(
        hProc,
        nullptr,
        0,
        (LPTHREAD_START_ROUTINE)loadLibA,
        lpBaseOfDll,
        0,
        nullptr
    );
    if (!hThread) INJECTOR_FAIL("CreateRemoteThread failed");

    WaitForSingleObject(hThread, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeThread(hThread, &exitCode);

    if (exitCode == 0) {
        cout << "LoadLibraryA failed inside target process! Possible GetLastError: ERROR_MOD_NOT_FOUND (DLL or dependency missing)\n";
    } else {
        cout << "DLL loaded successfully!\n";
    }

    CloseHandle(hThread);
    VirtualFreeEx(hProc, lpBaseOfDll, 0, MEM_RELEASE);
    CloseHandle(hProc);

    return 0;
}
