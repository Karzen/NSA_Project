#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>

DWORD GetPidByName(const std::wstring& processName){
    DWORD pid = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if(hSnapshot == INVALID_HANDLE_VALUE){
        std::cerr << "Failed to create snapshot. Error: " << GetLastError() << std::endl;
        return 0;
    }

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    if(Process32FirstW(hSnapshot, &processEntry)){
        do {
            if (processName == processEntry.szExeFile) {
                pid = processEntry.th32ProcessID;
                break;
            }

        } while (Process32NextW(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);

    return pid;
}


int main() {
    std::cout << "Injector initialized..." << std::endl;
    // Your first task: Use GetCurrentProcessId() to try opening your own process
    // as a safe test before targeting other apps.
    DWORD pid = GetCurrentProcessId();
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (hProcess) {
        std::cout << "Successfully opened process: " << pid << std::endl;
        CloseHandle(hProcess);
    } else {
        std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
    }

    return 0;
}