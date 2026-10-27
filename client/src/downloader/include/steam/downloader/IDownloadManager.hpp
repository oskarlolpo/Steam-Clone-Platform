#pragma once

#include "steam/downloader/DownloadTypes.hpp"
#include <functional>
#include <future>
#include <memory>
#include <string>

namespace steam::downloader {

using DownloadId = std::uint64_t;
using ProgressCallback = std::function<void(DownloadId id, const DownloadProgress& progress)>;
using StatusCallback = std::function<void(DownloadId id, DownloadStatus status, const std::string& message)>;

class IDownloadManager {
public:
    virtual ~IDownloadManager() = default;

    // Enqueues a new download task; returns a unique DownloadId
    [[nodiscard]] virtual DownloadId enqueue(DownloadRequest request) = 0;

    // Task lifecycle controls
    virtual void pause(DownloadId id) = 0;
    virtual void resume(DownloadId id) = 0;
    virtual void cancel(DownloadId id) = 0;

    // Global throttling & limits
    virtual void setMaxConcurrentDownloads(std::size_t limit) = 0;
    virtual void setBandwidthLimit(std::uint64_t bytesPerSec) = 0;

    // Status queries
    [[nodiscard]] virtual DownloadStatus getStatus(DownloadId id) const = 0;
    [[nodiscard]] virtual DownloadProgress getProgress(DownloadId id) const = 0;

    // Event hooks (thread-safe invocations guaranteed)
    virtual void setProgressCallback(ProgressCallback callback) = 0;
    virtual void setStatusCallback(StatusCallback callback) = 0;
};

} // namespace steam::downloader
