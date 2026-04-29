
#include "ProcUtils.h"

DWORD GetPidByName(const std::wstring &processName) {
    DWORD pid = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot. Error: " << GetLastError() << std::endl;
        return 0;
    }

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &processEntry)) {
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