
#include "../../include/HellsGate/HellsGate.h"


WORD HellsGate(const char* functionName){

    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");

    if(!hNtdll) { std::cerr<<"Failed to get handle of ntdll.dll"<<std::endl; return 0; }

    PBYTE pBase = (PBYTE)hNtdll;

    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pBase;

    if(pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { std::cerr<<"Invalid DOS signature"<<std::endl; return 0; }

    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(pBase + pDosHeader->e_lfanew);

    if(pNtHeaders->Signature != IMAGE_NT_SIGNATURE) { std::cerr<<"Invalid NT signature"<<std::endl; return 0; }

    IMAGE_DATA_DIRECTORY exportDir = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(pBase + exportDir.VirtualAddress);


    PDWORD pdwAddressOfNames = (PDWORD)(pBase + pExportDirectory->AddressOfNames);
    PDWORD pdwAddressOfFunctions = (PDWORD)(pBase + pExportDirectory->AddressOfFunctions);
    PWORD pwAddressOfNameOrdinals = (PWORD)(pBase + pExportDirectory->AddressOfNameOrdinals);

    WORD ssn = 0;
    for (DWORD i = 0; i < pExportDirectory->NumberOfNames; i++){

        const char* szFunctionName = (const char*)(pBase + pdwAddressOfNames[i]);

        if(strcmp(szFunctionName, functionName) == 0) {

            WORD wOrdinal = pwAddressOfNameOrdinals[i];

            PVOID pFunctionAddress = (PVOID)(pBase + pdwAddressOfFunctions[wOrdinal]);

            std::cout << "Found "<< functionName << " function at: " << pFunctionAddress << std::endl;

            unsigned char* pChars = (unsigned char*)pFunctionAddress;

            // The signature of a clean syscall:
            // 4C 8B D1    -> mov r10, rcx
            // B8 XX XX    -> mov eax, <SSN>
            if (pChars[0] == 0x4C && pChars[3] == 0xB8) {
                ssn = pChars[4];
                std::cout << "Successfully extracted SSN for "<< functionName <<" : 0x" << std::hex << ssn << std::endl;
            } else {
                std::cerr << "Function is hooked or modified! Cannot extract SSN." << std::endl;
                return 0;
            }
            break;
        }
    }

    if(ssn == 0) { std::cerr<<"Failed to find NtWriteVirtualMemory or extract SSN"<<std::endl; return 0; }

    return ssn;
}