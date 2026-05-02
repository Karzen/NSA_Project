//
// Created by Stefy on 5/2/2026.
//

#ifndef NSA_PROJECT_SYSCALLMANAGER_H
#define NSA_PROJECT_SYSCALLMANAGER_H


#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include "HellsGate/HellsGate.h"

// Define the generic assembly function with the maximum possible arguments
extern "C" NTSTATUS GenericInternalSyscall(
        PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, PVOID arg5,
        PVOID arg6, PVOID arg7, PVOID arg8, PVOID arg9, PVOID arg10,
        PVOID arg11,
        WORD ssn // The 12th argument
);

class SyscallManager {

public:
    SyscallManager(const SyscallManager&) = delete;
    void operator=(const SyscallManager&) = delete;

    static std::vector<std::wstring> functions;

    static SyscallManager& getInstance();

    NTSTATUS SyscallNtWriteVirtualMemory(HANDLE hProc, PVOID base, PVOID buf, SIZE_T size, PSIZE_T written);
    NTSTATUS SyscallNtCreateThreadEx(PHANDLE hThread, ACCESS_MASK access, PVOID objAttr, HANDLE hProc, PVOID start, PVOID arg, ULONG flags, ULONG_PTR zero, SIZE_T stack, SIZE_T maxStack, PVOID attrList);

    WORD GetSSNOfFunction(const std::string& functionName);

private:

    SyscallManager() = default;
    std::map<std::string, WORD> syscallMap;


};


#endif //NSA_PROJECT_SYSCALLMANAGER_H
