#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace steam::downloader {

struct DownloadProgress {
    std::uint64_t bytesDownloaded{0};
    std::uint64_t totalBytes{0};
    double currentSpeedBps{0.0};
    float progressFraction{0.0f}; // 0.0 - 1.0
};

enum class DownloadStatus : std::uint8_t {
    Idle,
    Queued,
    Connecting,
    Downloading,
    Paused,
    Completed,
    Verifying,
    Failed,
    Canceled
};

struct DownloadRequest {
    std::string url;
    std::filesystem::path destinationPath;
    std::string expectedSha256{};
    std::size_t parallelChunks{4};
    bool allowResume{true};
};

} // namespace steam::downloader
