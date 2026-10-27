#pragma once

#include "steam/core/Types.hpp"
#include "steam/launcher/ProcessTypes.hpp"
#include <functional>
#include <memory>
#include <string>

namespace steam::launcher {

using ProcessStateCallback = std::function<void(steam::core::AppId appId, ProcessState state, int exitCode)>;
using PlaytimeCallback = std::function<void(steam::core::AppId appId, std::chrono::seconds sessionDuration)>;

class IProcessLauncher {
public:
    virtual ~IProcessLauncher() = default;

    // Spawns process asynchronously, monitors execution and lifetime
    virtual bool launch(steam::core::AppId appId, const LaunchConfig& config) = 0;

    // Terminate running application gracefully or forcefully
    virtual bool terminate(steam::core::AppId appId, bool force = false) = 0;

    // State queries
    [[nodiscard]] virtual bool isRunning(steam::core::AppId appId) const = 0;
    [[nodiscard]] virtual ProcessExecutionInfo getInfo(steam::core::AppId appId) const = 0;

    // Notification observers
    virtual void setProcessStateCallback(ProcessStateCallback callback) = 0;
    virtual void setPlaytimeCallback(PlaytimeCallback callback) = 0;
};

} // namespace steam::launcher
