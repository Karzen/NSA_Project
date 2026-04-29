//
// Created by Stefy on 4/29/2026.
//

#ifndef NSA_PROJECT_FILESYSTEMUTILS_H
#define NSA_PROJECT_FILESYSTEMUTILS_H


#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

#include <shlobj.h> // Required for SHGetKnownFolderPath
#include <combaseapi.h> // Required for CoTaskMemFree

#include "library_data.h" // Generated header containing the binary data of the library

std::wstring GetAbsolutePathOfFile(std::wstring relativeFileName);

std::wstring GetSelfFolder();

std::wstring GetSelfPath();

std::wstring GetStartupFolder();

bool CopyFileToFolder(const std::wstring& src, const std::wstring& dest);

void ExtractLibrary();

#endif //NSA_PROJECT_FILESYSTEMUTILS_H
