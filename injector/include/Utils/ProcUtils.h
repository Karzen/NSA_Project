//
// Created by Stefy on 4/29/2026.
//

#ifndef NSA_PROJECT_PROCUTILS_H
#define NSA_PROJECT_PROCUTILS_H

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>


DWORD GetPidByName(const std::wstring &processName);

#endif //NSA_PROJECT_PROCUTILS_H
