#include "steam/downloader/ArchiveExtractor.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace steam::downloader {

bool ArchiveExtractor::extractOrDeploy(
    const std::filesystem::path& archivePath,
    const std::filesystem::path& targetDir,
    ExtractionProgressCallback progressCallback
) {
    std::error_code ec;
    std::filesystem::create_directories(targetDir, ec);
    if (ec) {
        return false;
    }

    if (!std::filesystem::exists(archivePath)) {
        return false;
    }

    const std::string ext = archivePath.extension().string();

    if (ext == ".zip") {
#ifdef _WIN32
        // Use Windows native tar or powershell Expand-Archive
        std::wstring cmd = L"tar.exe -xf \"" + archivePath.wstring() + L"\" -C \"" + targetDir.wstring() + L"\"";
        STARTUPINFOW si{};
        PROCESS_INFORMATION pi{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD exitCode = 0;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            if (exitCode == 0) {
                if (progressCallback) progressCallback(1.0f, "Completed");
                return true;
            }
        }
#endif
    }

    // Deploy payload or executable directly to target if already raw binary / depot chunk
    const auto destinationFile = targetDir / archivePath.filename();
    if (archivePath != destinationFile) {
        std::filesystem::copy_file(archivePath, destinationFile, std::filesystem::copy_options::overwrite_existing, ec);
    }

    if (progressCallback) {
        progressCallback(1.0f, destinationFile.filename().string());
    }

    return !ec;
}

} // namespace steam::downloader
