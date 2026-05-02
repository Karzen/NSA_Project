//
// Created by Stefy on 4/29/2026.
//

#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibrarySyscall.h"



bool InjectProcessSyscall(const DWORD pid, const std::wstring dllPath) {

    SIZE_T dllPathSize = dllPath.size() * sizeof(wchar_t) + sizeof(wchar_t);

    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, pid);

    if (hProcess == NULL) {
        std::cerr << "Failed to open target process. Error: " << GetLastError() << std::endl;
        return false;
    }

    LPVOID externalAddress = VirtualAllocEx(
            hProcess,
            NULL,
            dllPathSize,
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE
    );

    if (externalAddress == NULL) {
        std::cerr << "Failed to allocate memory in target process. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    SIZE_T bytesWritten;


    NTSTATUS status = SyscallManager::getInstance().SyscallNtWriteVirtualMemory(
            hProcess,
            externalAddress,
            (PVOID)dllPath.c_str(),
            dllPathSize,
            &bytesWritten
    );

//    if (bytesWritten != dllPathSize) {
//        std::cerr << "Failed to write full DLL path to target process. " << bytesWritten << " " << dllPathSize
//                  << " Error: " << GetLastError() << std::endl;
//        VirtualFreeEx(hProcess, externalAddress, 0, MEM_RELEASE);
//        CloseHandle(hProcess);
//        return false;
//    }

    auto loadLibraryWAddress = (LPVOID) GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");

    if (loadLibraryWAddress == NULL) {
        std::cerr << "Failed to get address of LoadLibraryW. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, externalAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hTread = NULL;
    SyscallManager::getInstance().SyscallNtCreateThreadEx(
            &hTread,                                    // Thread handle (output)
            THREAD_ALL_ACCESS,                           // Desired access
            NULL,                                        // Object attributes
            hProcess,                                    // Target process handle
            (PVOID)loadLibraryWAddress, // Start routine
            externalAddress,                             // Argument (DLL path)
            0,                                           // CreateFlags (0 = normal)
            0,                                           // ZeroBits
            0,                                           // Stack size (default)
            0,                                           // Maximum stack size
            NULL                                       // Attribute list
    );

    if (hTread == NULL) {
        std::cerr << "Failed to create remote thread. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, externalAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }


    WaitForSingleObject(hTread, INFINITE);


    VirtualFreeEx(hProcess, externalAddress, 0, MEM_RELEASE);
    CloseHandle(hTread);
    CloseHandle(hProcess);

    std::cout <<"DLL injected successfully!" << std::endl;

    return true;
}