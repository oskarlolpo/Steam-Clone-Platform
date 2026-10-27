#pragma once

#include "steam/downloader/IDownloadManager.hpp"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <thread>
#include <vector>

namespace steam::downloader {

struct ChunkInfo {
    std::size_t chunkIndex{0};
    std::uint64_t startByte{0};
    std::uint64_t endByte{0};
    std::uint64_t downloadedBytes{0};
    bool isCompleted{false};
};

struct ActiveTask {
    DownloadId id{0};
    DownloadRequest request;
    DownloadStatus status{DownloadStatus::Idle};
    DownloadProgress progress{};
    std::vector<ChunkInfo> chunks;
    std::chrono::steady_clock::time_point lastSpeedCheckTime{};
    std::uint64_t bytesSinceLastSpeedCheck{0};
    bool cancelRequested{false};
    bool pauseRequested{false};
};

class DownloadManager final : public IDownloadManager {
public:
    explicit DownloadManager(std::size_t workerThreads = 4);
    ~DownloadManager() override;

    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;
    DownloadManager(DownloadManager&&) = delete;
    DownloadManager& operator=(DownloadManager&&) = delete;

    [[nodiscard]] DownloadId enqueue(DownloadRequest request) override;

    void pause(DownloadId id) override;
    void resume(DownloadId id) override;
    void cancel(DownloadId id) override;

    void setMaxConcurrentDownloads(std::size_t limit) override;
    void setBandwidthLimit(std::uint64_t bytesPerSec) override;

    [[nodiscard]] DownloadStatus getStatus(DownloadId id) const override;
    [[nodiscard]] DownloadProgress getProgress(DownloadId id) const override;

    void setProgressCallback(ProgressCallback callback) override;
    void setStatusCallback(StatusCallback callback) override;

private:
    void workerLoop(std::stop_token stopToken);
    void executeTask(ActiveTask& task, std::stop_token stopToken);
    bool downloadChunk(ActiveTask& task, ChunkInfo& chunk, std::stop_token stopToken);
    bool verifyChecksum(const ActiveTask& task);
    void updateProgress(ActiveTask& task, std::uint64_t newBytes);

    std::atomic<DownloadId> m_nextDownloadId{1};
    std::atomic<std::size_t> m_maxConcurrent{2};
    std::atomic<std::uint64_t> m_bandwidthLimitBps{0}; // 0 = unlimited

    mutable std::mutex m_mutex;
    std::condition_variable_any m_cv;
    std::deque<DownloadId> m_queue;
    std::map<DownloadId, std::shared_ptr<ActiveTask>> m_tasks;

    ProgressCallback m_progressCallback;
    StatusCallback m_statusCallback;

    std::vector<std::jthread> m_workers;
    std::atomic<bool> m_running{true};
};

} // namespace steam::downloader
