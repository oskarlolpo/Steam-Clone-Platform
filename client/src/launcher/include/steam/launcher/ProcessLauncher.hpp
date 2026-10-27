#pragma once

#include "steam/launcher/IProcessLauncher.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>

namespace steam::launcher {

struct MonitoredProcess {
    steam::core::AppId appId{0};
    LaunchConfig config;
    ProcessExecutionInfo info;
    std::jthread monitorThread;
#ifdef _WIN32
    void* processHandle{nullptr};
#endif
};

class ProcessLauncher final : public IProcessLauncher {
public:
    ProcessLauncher();
    ~ProcessLauncher() override;

    ProcessLauncher(const ProcessLauncher&) = delete;
    ProcessLauncher& operator=(const ProcessLauncher&) = delete;
    ProcessLauncher(ProcessLauncher&&) = delete;
    ProcessLauncher& operator=(ProcessLauncher&&) = delete;

    bool launch(steam::core::AppId appId, const LaunchConfig& config) override;
    bool terminate(steam::core::AppId appId, bool force = false) override;

    [[nodiscard]] bool isRunning(steam::core::AppId appId) const override;
    [[nodiscard]] ProcessExecutionInfo getInfo(steam::core::AppId appId) const override;

    void setProcessStateCallback(ProcessStateCallback callback) override;
    void setPlaytimeCallback(PlaytimeCallback callback) override;

private:
    void monitorWorker(steam::core::AppId appId, std::stop_token stopToken);

    mutable std::mutex m_mutex;
    std::map<steam::core::AppId, std::shared_ptr<MonitoredProcess>> m_processes;
    ProcessStateCallback m_stateCallback;
    PlaytimeCallback m_playtimeCallback;
};

} // namespace steam::launcher
