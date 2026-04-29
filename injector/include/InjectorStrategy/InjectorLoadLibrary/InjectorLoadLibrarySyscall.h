//
// Created by Stefy on 4/29/2026.
//

#ifndef NSA_PROJECT_INJECTORLOADLIBRARYSYSCALL_H
#define NSA_PROJECT_INJECTORLOADLIBRARYSYSCALL_H

#include <windows.h>
#include <ntdef.h>
#include <iostream>
#include "HellsGate/HellsGate.h"

bool InjectProcessSyscall(const DWORD pid, const std::wstring dllPath);

#endif //NSA_PROJECT_INJECTORLOADLIBRARYSYSCALL_H
