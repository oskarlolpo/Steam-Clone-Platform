#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace steam::launcher {

using ProcessId = std::uint32_t;

enum class ProcessState : std::uint8_t {
    NotRunning,
    Launching,
    Running,
    Exited,
    Crashed
};

struct LaunchConfig {
    std::filesystem::path executablePath;
    std::filesystem::path workingDirectory;
    std::vector<std::string> arguments;
    std::vector<std::pair<std::string, std::string>> environmentVariables;
    bool createNewConsole{true};
};

struct ProcessExecutionInfo {
    ProcessId pid{0};
    ProcessState state{ProcessState::NotRunning};
    int exitCode{0};
    std::chrono::system_clock::time_point startTime{};
    std::chrono::system_clock::time_point endTime{};
    std::chrono::seconds totalPlaytime{0};
};

} // namespace steam::launcher
