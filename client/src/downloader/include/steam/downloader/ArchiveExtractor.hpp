#pragma once

#include <filesystem>
#include <functional>
#include <string>

namespace steam::downloader {

using ExtractionProgressCallback = std::function<void(float progress, const std::string& currentFile)>;

class ArchiveExtractor {
public:
    ArchiveExtractor() = default;

    // Decompresses/deploys payload into target directory
    [[nodiscard]] static bool extractOrDeploy(
        const std::filesystem::path& archivePath,
        const std::filesystem::path& targetDir,
        ExtractionProgressCallback progressCallback = nullptr
    );
};

} // namespace steam::downloader
