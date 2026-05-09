//
// Created by Stefy on 5/2/2026.
//

#ifndef NSA_PROJECT_INJECTORMANUALMAP_H
#define NSA_PROJECT_INJECTORMANUALMAP_H

#include <ntdef.h>
#include "HellsGate/SyscallManager.h"
#include "Utils/FilesystemUtils.h"


bool InjectManualMap(const DWORD pid, const std::wstring dllPath);

#endif //NSA_PROJECT_INJECTORMANUALMAP_H
