//
// Created by Stefy on 5/2/2026.
//

#include "InjectorStrategy/InjectorManualMap/InjectorManualMap.h"


PIMAGE_NT_HEADERS GetNtHeader(PBYTE &pLocalBase) {
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER) pLocalBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "Invalid DOS header signature." << std::endl;
        return nullptr;
    }

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS) (pLocalBase + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "Invalid NT header signature." << std::endl;
        return nullptr;
    }
    return pNtHeaders;
}

DWORD RvaToOffset(DWORD rva, PIMAGE_NT_HEADERS pNtHeaders) {
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);

    for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        // Check if the RVA falls inside this specific section
        if (rva >= pSectionHeader[i].VirtualAddress &&
            rva < (pSectionHeader[i].VirtualAddress + pSectionHeader[i].SizeOfRawData)) {

            // Calculate the exact offset in our raw file buffer
            return rva - pSectionHeader[i].VirtualAddress + pSectionHeader[i].PointerToRawData;
        }
    }
    return rva;
}

void FixReloc(HANDLE hProcess, PBYTE &pLocalBase, LPVOID &externalAddress) {


    PIMAGE_NT_HEADERS pNtHeaders = GetNtHeader(pLocalBase);

    ULONG_PTR preferredBase = pNtHeaders->OptionalHeader.ImageBase;
    ULONG_PTR delta = (ULONG_PTR) externalAddress - preferredBase;

    std::cout << "DLL Preferred Base: 0x" << std::hex << preferredBase << std::endl;
    std::cout << "DLL Allocated External Address: 0x" << std::hex << externalAddress << std::endl;
    std::cout << "Calculated Delta: 0x" << std::hex << delta << std::endl;

    if (delta != 0) {

        auto relocDir = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

        if (relocDir.Size > 0) {
            PIMAGE_BASE_RELOCATION pReloc = (PIMAGE_BASE_RELOCATION) (pLocalBase +
                                                                      RvaToOffset(relocDir.VirtualAddress, pNtHeaders));

            while (pReloc->VirtualAddress != 0) {
                DWORD entryCount = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                PWORD pRelocEntry = (PWORD) ((PBYTE) pReloc + sizeof(IMAGE_BASE_RELOCATION));

                for (DWORD i = 0; i < entryCount; i++) {
                    WORD type = pRelocEntry[i] >> 12;
                    WORD offset = pRelocEntry[i] & 0xFFF;

                    if (type == IMAGE_REL_BASED_DIR64) {
                        ULONG_PTR pointerRva = pReloc->VirtualAddress + offset;

                        ULONG_PTR *pOriginalPointer = (ULONG_PTR *) (pLocalBase + RvaToOffset(pointerRva, pNtHeaders));

                        ULONG_PTR patchedPointer = *pOriginalPointer + delta;


                        PVOID pRemotePointerAddress = (PBYTE) externalAddress + pointerRva;
                        SIZE_T bytesWritten = 0;

                        SyscallManager::getInstance().SyscallNtWriteVirtualMemory(
                                hProcess,
                                pRemotePointerAddress,
                                &patchedPointer,
                                sizeof(ULONG_PTR),
                                &bytesWritten
                        );
                    }
                }
                pReloc = (PIMAGE_BASE_RELOCATION) ((PBYTE) pReloc + pReloc->SizeOfBlock);
            }
            std::cout << "Relocations fixed successfully!" << std::endl;
        }
    }
}

void ResolveIAT(HANDLE hProcess, PBYTE &pLocalBase, LPVOID &externalAddress) {

    PIMAGE_NT_HEADERS pNtHeaders = GetNtHeader(pLocalBase);

    IMAGE_DATA_DIRECTORY importDir = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (importDir.Size > 0) {

        PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR) (pLocalBase +
                                                                           RvaToOffset(importDir.VirtualAddress,
                                                                                       pNtHeaders));

        while (pImportDesc->Name != 0) {
            LPCSTR dllName = (LPCSTR) (pLocalBase + RvaToOffset(pImportDesc->Name, pNtHeaders));

            HMODULE hModule = LoadLibraryA(dllName);

            if (!hModule) {
                std::cerr << "Failed to load dependency DLL locally: " << dllName << std::endl;
                return;
            }
            std::cout << "Resolving imports for: " << dllName << std::endl;

            PIMAGE_THUNK_DATA pOriginalFirstThunk = (PIMAGE_THUNK_DATA) (pLocalBase +
                                                                         RvaToOffset(pImportDesc->OriginalFirstThunk,
                                                                                     pNtHeaders));
            PIMAGE_THUNK_DATA pFirstThunk = (PIMAGE_THUNK_DATA) (pLocalBase +
                                                                 RvaToOffset(pImportDesc->FirstThunk, pNtHeaders));


            ULONG_PTR iatRvaOffset = pImportDesc->FirstThunk;


            while (pOriginalFirstThunk->u1.AddressOfData != 0) {

                FARPROC functionAddress = NULL;

                if (IMAGE_SNAP_BY_ORDINAL(pOriginalFirstThunk->u1.Ordinal)) {

                    WORD ordinal = IMAGE_ORDINAL(pOriginalFirstThunk->u1.Ordinal);
                    functionAddress = GetProcAddress(hModule, (LPCSTR) ordinal);
                } else {
                    PIMAGE_IMPORT_BY_NAME pImportByName = (PIMAGE_IMPORT_BY_NAME) (pLocalBase + RvaToOffset(
                            pOriginalFirstThunk->u1.AddressOfData, pNtHeaders));
                    LPCSTR functionName = (LPCSTR) pImportByName->Name;
                    functionAddress = GetProcAddress(hModule, functionName);
                }


                if (functionAddress != NULL) {


                    PVOID pRemoteIatSlot = (PBYTE) externalAddress + iatRvaOffset;
                    SIZE_T bytesWritten = 0;


                    SyscallManager::getInstance().SyscallNtWriteVirtualMemory(
                            hProcess,
                            pRemoteIatSlot,
                            &functionAddress,
                            sizeof(functionAddress),
                            &bytesWritten
                    );
                } else {
                    std::cerr << "[-] Failed to resolve function address!" << std::endl;
                }

                pOriginalFirstThunk++;
                pFirstThunk++;

                iatRvaOffset += sizeof(IMAGE_THUNK_DATA);
            }
            pImportDesc++;
        }

        std::cout << "IAT resolved successfully!" << std::endl;
    }
}

bool RunMappedDll(HANDLE hProcess, PBYTE &pLocalBase, LPVOID &externalAddress){

    PIMAGE_NT_HEADERS pNtHeaders = GetNtHeader(pLocalBase);

    DWORD entryPointRva = pNtHeaders->OptionalHeader.AddressOfEntryPoint;

    if (entryPointRva == 0) {
        std::cerr << "[-] Error: DLL has no Entry Point!" << std::endl;
        return false;
    }

    PVOID pRemoteEntryPoint = (PBYTE)externalAddress + entryPointRva;
    std::cout << "[+] Remote Entry Point calculated at: " << pRemoteEntryPoint << std::endl;

    HANDLE hRemoteThread = NULL;

    SyscallManager::getInstance().SyscallNtCreateThreadEx(
            &hRemoteThread,
            THREAD_ALL_ACCESS,
            NULL,
            hProcess,
            pRemoteEntryPoint,
            externalAddress,
            0x4,
            0,
            0,
            0,
            NULL
    );

// 5. Check the result
    if (hRemoteThread != NULL) {
        std::cout << "[+] Thread started! Waiting for execution..." << std::endl;

        // Wait for the payload to finish executing
        WaitForSingleObject(hRemoteThread, INFINITE);

        // THE STETHOSCOPE: How did the thread die?
        DWORD exitCode = 0;
        GetExitCodeThread(hRemoteThread, &exitCode);

        if (exitCode == 0xC0000005) {
            std::cerr << "[-] Thread crashed with ACCESS VIOLATION (0xC0000005)" << std::endl;
        } else {
            std::cout << "[+] Thread exited with code: 0x" << std::hex << exitCode << std::endl;
        }

        CloseHandle(hRemoteThread);
    }


    CloseHandle(hProcess);
    return true;
}



void CopyDllBufferToExProcessAddress(HANDLE hProcess, std::vector<BYTE> &dllData, PBYTE &pLocalBase,
                                     LPVOID &externalAddress) {
    pLocalBase = dllData.data();

    PIMAGE_NT_HEADERS pNtHeaders = GetNtHeader(pLocalBase);

    SIZE_T sizeOfImage = pNtHeaders->OptionalHeader.SizeOfImage;
    std::cout << "Size of image: " << sizeOfImage << std::endl;

    externalAddress = NULL; // Initialize the external address

    NTSTATUS allocStatus = SyscallManager::getInstance().SyscallNtAllocateVirtualMemory(
            hProcess,
            &externalAddress, // Pass the address of the base address
            0,                // ZeroBits (usually 0)
            &sizeOfImage,      // Pass the address of the region size
            MEM_COMMIT | MEM_RESERVE,
            PAGE_EXECUTE_READWRITE
    );

    if (!NT_SUCCESS(allocStatus)) { // Check if the allocation was successful
        std::cerr << "Failed to allocate memory in target process. NTSTATUS: " << allocStatus << std::endl;
        CloseHandle(hProcess);
        return;
    }

    SIZE_T bytesWritten = 0;

    NTSTATUS status = SyscallManager::getInstance().SyscallNtWriteVirtualMemory(
            hProcess,
            externalAddress,
            pLocalBase,
            pNtHeaders->OptionalHeader.SizeOfHeaders,
            &bytesWritten
    );

    if (status != 0) {
        std::cerr << "Failed to write PE Headers. Status: 0x" << std::hex << status << std::endl;
        return;
    }
    std::cout << "PE Headers written successfully." << std::endl;

    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);

    for (int i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        if (pSectionHeader[i].SizeOfRawData > 0) {
            PVOID pDestination = (PBYTE) externalAddress + pSectionHeader[i].VirtualAddress;
            PVOID pSource = pLocalBase + pSectionHeader[i].PointerToRawData;
            SIZE_T size = pSectionHeader[i].SizeOfRawData;

            status = SyscallManager::getInstance().SyscallNtWriteVirtualMemory(
                    hProcess,
                    pDestination,
                    pSource,
                    size,
                    &bytesWritten
            );

            if (status == 0) {
                std::cout << "Mapped Section: " << pSectionHeader[i].Name
                          << " to " << pDestination << std::endl;
            } else {
                std::cerr << "Failed to map section: " << pSectionHeader[i].Name << std::endl;
                return;
            }
        }
    }
}

bool InjectManualMap(const DWORD pid, const std::wstring dllPath) {

    std::cout << "Beginning manual map injection on process pid=" << pid << std::endl;

    auto dllData = ReadFileToBuffer(dllPath);

    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, FALSE, pid);

    PBYTE pLocalBase = NULL;
    LPVOID externalAddress = NULL;

    CopyDllBufferToExProcessAddress(hProcess, dllData, pLocalBase, externalAddress);
    FixReloc(hProcess, pLocalBase, externalAddress);
    ResolveIAT(hProcess, pLocalBase, externalAddress);
    RunMappedDll(hProcess, pLocalBase, externalAddress);

    system("pause");


    return true;
}