//
// Created by Stefy on 4/29/2026.
//

#ifndef NSA_PROJECT_INJECTORLOADLIBRARYUNDOCUMENTED_H
#define NSA_PROJECT_INJECTORLOADLIBRARYUNDOCUMENTED_H

#include <windows.h>
#include <ntdef.h>
#include <iostream>


bool InjectProcessUndocumented(const DWORD pid, const std::wstring dllPath);

#endif //NSA_PROJECT_INJECTORLOADLIBRARYUNDOCUMENTED_H
