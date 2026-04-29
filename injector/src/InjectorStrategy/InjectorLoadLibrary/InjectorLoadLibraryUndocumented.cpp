//
// Created by Stefy on 4/29/2026.
//

#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryDefault.h"


HANDLE RunWithNtCreateThread(HANDLE hProcess, LPVOID loadLibraryWAddress, LPVOID externalAddress) {

    typedef NTSTATUS (NTAPI *pNtCreateThreadEx)(
            PHANDLE                 ThreadHandle,
            ACCESS_MASK             DesiredAccess,
            POBJECT_ATTRIBUTES      ObjectAttributes,
            HANDLE                  ProcessHandle,
            LPTHREAD_START_ROUTINE  StartRoutine,
            LPVOID                  Argument,
            ULONG                   CreateFlags,
            SIZE_T                  ZeroBits,
            SIZE_T                  StackSize,
            SIZE_T                  MaximumStackSize,
            LPVOID                  AttributeList
    );

    auto NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(
            GetModuleHandle(TEXT("ntdll.dll")),
            "NtCreateThreadEx"
    );

    if (!NtCreateThreadEx) {
        std::cerr<<"Failed to get NtCreateThreadEx"<<std::endl;
        return NULL;
    }

    HANDLE hThread = NULL;
    auto status = NtCreateThreadEx(
            &hThread,                                    // Thread handle (output)
            THREAD_ALL_ACCESS,                           // Desired access
            NULL,                                        // Object attributes
            hProcess,                                    // Target process handle
            (LPTHREAD_START_ROUTINE)loadLibraryWAddress, // Start routine
            externalAddress,                             // Argument (DLL path)
            0,                                           // CreateFlags (0 = normal)
            0,                                           // ZeroBits
            0,                                           // Stack size (default)
            0,                                           // Maximum stack size
            NULL                                         // Attribute list
    );

    if (status != 0 || hThread == NULL) {
        std::cerr<<"NtCreateThreadEx failed with status: "<< status << std::endl;
        return NULL;
    }

    return hThread;
}

SIZE_T RunWithNtWriteVirtualMemory(HANDLE hProcess, LPVOID baseAddress, LPVOID buffer, SIZE_T size) {

    typedef NTSTATUS (NTAPI *pNtWriteVirtualMemory)(
            HANDLE ProcessHandle,
            PVOID BaseAddress,
            PVOID Buffer,
            SIZE_T NumberOfBytesToWrite,
            PSIZE_T NumberOfBytesWritten
    );

    auto NtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(
            GetModuleHandle(TEXT("ntdll.dll")),
            "NtWriteVirtualMemory"
    );

    if (!NtWriteVirtualMemory) {
        std::cerr<<"Failed to get NtWriteVirtualMemory"<<std::endl;
        return NULL;
    }

    SIZE_T bytesWritten = 0;
    auto status = NtWriteVirtualMemory(
            hProcess,
            baseAddress,
            buffer,
            size,
            &bytesWritten
    );

    if (status != 0) {
        std::cerr<<"NtWriteVirtualMemory failed with status: "<< status << std::endl;
        return NULL;
    }

    return bytesWritten;
}

LPVOID RunWithNtAllocateVirtualMemory(HANDLE hProcess, LPVOID baseAddress, SIZE_T size, DWORD allocationType, DWORD protect) {

    typedef NTSTATUS (NTAPI *pNtAllocateVirtualMemory)(
            HANDLE ProcessHandle,
            PVOID *BaseAddress,        // pointer to pointer (output)
            ULONG_PTR ZeroBits,
            PSIZE_T RegionSize,        // pointer to SIZE_T (input/output)
            ULONG AllocationType,
            ULONG Protect
    );

    auto NtAllocateVirtualMemory = (pNtAllocateVirtualMemory)GetProcAddress(
            GetModuleHandle(TEXT("ntdll.dll")),
            "NtAllocateVirtualMemory"
    );

    if (!NtAllocateVirtualMemory) {
        std::cerr << "Failed to get NtAllocateVirtualMemory" << std::endl;
        return NULL;
    }

    // Set up input parameters
    PVOID allocatedAddress = baseAddress;   // may be NULL for let system choose
    SIZE_T regionSize = size;

    NTSTATUS status = NtAllocateVirtualMemory(
            hProcess,
            &allocatedAddress,    // receives the actual address
            0,                    // ZeroBits
            &regionSize,          // receives actual allocated size (should equal size)
            allocationType,
            protect
    );

    if (status != 0) {
        std::cerr << "NtAllocateVirtualMemory failed with status: 0x" << std::hex << status << std::endl;
        return NULL;
    }

    return allocatedAddress;
}


bool InjectProcessUndocumented(const DWORD pid, const std::wstring dllPath) {

    SIZE_T dllPathSize = dllPath.size() * sizeof(wchar_t) + sizeof(wchar_t);

    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, pid);

    if (hProcess == NULL) {
        std::cerr << "Failed to open target process. Error: " << GetLastError() << std::endl;
        return false;
    }

    LPVOID externalAddress = RunWithNtAllocateVirtualMemory(
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


    SIZE_T bytesWritten = RunWithNtWriteVirtualMemory(hProcess, externalAddress, (PVOID)dllPath.c_str(), dllPathSize);


    if (bytesWritten != dllPathSize) {
        std::cerr << "Failed to write full DLL path to target process. " << bytesWritten << " " << dllPathSize
                  << " Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, externalAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    auto loadLibraryWAddress = (LPVOID) GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");

    if (loadLibraryWAddress == NULL) {
        std::cerr << "Failed to get address of LoadLibraryW. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, externalAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hTread = RunWithNtCreateThread(hProcess, loadLibraryWAddress, externalAddress);


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