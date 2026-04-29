//
// Created by Stefy on 4/29/2026.
//

#include "Utils/FilesystemUtils.h"

namespace fs = std::filesystem;

std::wstring GetAbsolutePathOfFile(std::wstring relativeFileName){
    fs::path filePath =  std::filesystem::absolute(relativeFileName);
    return filePath.wstring();
}

std::wstring GetSelfPath() {
    wchar_t buffer[MAX_PATH];
    // Get the full path of the current process .exe
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    return std::wstring(buffer);
}

std::wstring GetSelfFolder() {
    std::wstring fullPath = GetSelfPath();
    // Use filesystem to get just the directory portion
    return fs::path(fullPath).parent_path().wstring();
}

std::wstring GetStartupFolder(){
    PWSTR pszPath = NULL;

    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Startup, 0, NULL, &pszPath);

    if (SUCCEEDED(hr)) {
        std::wcout << L"Startup Folder: " << pszPath << std::endl;
        auto startupPath = std::wstring(pszPath);
        CoTaskMemFree(pszPath);
        return startupPath;
    } else {
        std::cerr << "Failed to retrieve startup folder path. HRESULT: " << hr << std::endl;
        return L"";
    }
}

bool CopyFileToFolder(const std::wstring& src, const std::wstring& dest) {
    try {
        fs::path sourcePath(src);
        fs::path destDir(dest);

        if (!fs::exists(sourcePath) || !fs::is_regular_file(sourcePath)) {
            std::wcerr << L"Source file does not exist: " << src << std::endl;
            return false;
        }

        if (!fs::exists(destDir)) {
            fs::create_directories(destDir);
        }

        fs::path targetPath = destDir / sourcePath.filename();

        return fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem Error: " << e.what() << std::endl;
        return false;
    }
}

void ExtractLibrary() {
    std::string filename = "library.dll";

    // Only extract if it's not already there
    if (!std::filesystem::exists(filename)) {
        std::ofstream outfile(filename, std::ios::binary);
        outfile.write(reinterpret_cast<const char*>(raw_dll), raw_dll_len);
        outfile.close();
        std::cout << "Library extracted to " << filename << std::endl;
    }
}