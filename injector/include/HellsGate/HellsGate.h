
#ifndef NSA_PROJECT_HELLSGATE_H
#define NSA_PROJECT_HELLSGATE_H

#include <windows.h>
#include <iostream>

// Define the generic assembly function with the maximum possible arguments
extern "C" NTSTATUS GenericInternalSyscall(
        PVOID arg1, PVOID arg2, PVOID arg3, PVOID arg4, PVOID arg5,
        PVOID arg6, PVOID arg7, PVOID arg8, PVOID arg9, PVOID arg10,
        PVOID arg11,
        WORD ssn // The 12th argument
);

WORD HellsGate(const char* functionName);

#endif //NSA_PROJECT_HELLSGATE_H
