#include "steam/launcher/ProcessLauncher.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace steam::launcher {

ProcessLauncher::ProcessLauncher() = default;

ProcessLauncher::~ProcessLauncher() {
    std::scoped_lock lock(m_mutex);
    for (auto& [appId, proc] : m_processes) {
#ifdef _WIN32
        if (proc->processHandle) {
            CloseHandle(static_cast<HANDLE>(proc->processHandle));
            proc->processHandle = nullptr;
        }
#endif
    }
}

bool ProcessLauncher::launch(steam::core::AppId appId, const LaunchConfig& config) {
    std::scoped_lock lock(m_mutex);

    if (auto it = m_processes.find(appId); it != m_processes.end()) {
        if (it->second->info.state == ProcessState::Running) {
            return false; // Already running
        }
    }

    auto proc = std::make_shared<MonitoredProcess>();
    proc->appId = appId;
    proc->config = config;
    proc->info.state = ProcessState::Launching;
    proc->info.startTime = std::chrono::system_clock::now();

#ifdef _WIN32
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::wostringstream cmdLine;
    cmdLine << L"\"" << config.executablePath.wstring() << L"\"";
    for (const auto& arg : config.arguments) {
        std::wstring warg(arg.begin(), arg.end());
        if (arg.find(' ') != std::string::npos && arg.front() != '"') {
            cmdLine << L" \"" << warg << L"\"";
        } else {
            cmdLine << L" " << warg;
        }
    }

    std::wstring cmdStr = cmdLine.str();
    std::wstring workingDirStr = config.workingDirectory.empty() 
        ? config.executablePath.parent_path().wstring() 
        : config.workingDirectory.wstring();

    const DWORD creationFlags = config.createNewConsole ? CREATE_NEW_CONSOLE : 0;
    const BOOL success = CreateProcessW(
        nullptr,
        cmdStr.data(),
        nullptr,
        nullptr,
        FALSE,
        creationFlags,
        nullptr,
        workingDirStr.empty() ? nullptr : workingDirStr.c_str(),
        &si,
        &pi
    );

    if (!success) {
        proc->info.state = ProcessState::Crashed;
        proc->info.exitCode = static_cast<int>(GetLastError());
        if (m_stateCallback) {
            m_stateCallback(appId, ProcessState::Crashed, proc->info.exitCode);
        }
        return false;
    }

    CloseHandle(pi.hThread); // Thread handle not needed for waiting
    proc->processHandle = pi.hProcess;
    proc->info.pid = static_cast<ProcessId>(pi.dwProcessId);
    proc->info.state = ProcessState::Running;
#else
    // POSIX fallback simulation
    proc->info.pid = 12345;
    proc->info.state = ProcessState::Running;
#endif

    m_processes[appId] = proc;

    if (m_stateCallback) {
        m_stateCallback(appId, ProcessState::Running, 0);
    }

    // Launch dedicated background monitoring thread
    proc->monitorThread = std::jthread([this, appId](std::stop_token st) {
        monitorWorker(appId, st);
    });

    return true;
}

void ProcessLauncher::monitorWorker(steam::core::AppId appId, std::stop_token stopToken) {
    std::shared_ptr<MonitoredProcess> proc;
    {
        std::scoped_lock lock(m_mutex);
        if (auto it = m_processes.find(appId); it != m_processes.end()) {
            proc = it->second;
        }
    }

    if (!proc) {
        return;
    }

    const auto startSteady = std::chrono::steady_clock::now();

    while (!stopToken.stop_requested()) {
#ifdef _WIN32
        if (proc->processHandle) {
            const DWORD waitResult = WaitForSingleObject(static_cast<HANDLE>(proc->processHandle), 500);
            if (waitResult == WAIT_OBJECT_0) {
                DWORD exitCode = 0;
                GetExitCodeProcess(static_cast<HANDLE>(proc->processHandle), &exitCode);
                CloseHandle(static_cast<HANDLE>(proc->processHandle));
                proc->processHandle = nullptr;

                const auto endSteady = std::chrono::steady_clock::now();
                const auto duration = std::chrono::duration_cast<std::chrono::seconds>(endSteady - startSteady);

                {
                    std::scoped_lock lock(m_mutex);
                    proc->info.state = (exitCode == 0) ? ProcessState::Exited : ProcessState::Crashed;
                    proc->info.exitCode = static_cast<int>(exitCode);
                    proc->info.endTime = std::chrono::system_clock::now();
                    proc->info.totalPlaytime = duration;
                }

                if (m_playtimeCallback) {
                    m_playtimeCallback(appId, duration);
                }

                if (m_stateCallback) {
                    m_stateCallback(appId, proc->info.state, proc->info.exitCode);
                }
                return;
            }
        }
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
    }
}

bool ProcessLauncher::terminate(steam::core::AppId appId, bool force) {
    std::scoped_lock lock(m_mutex);
    auto it = m_processes.find(appId);
    if (it == m_processes.end() || it->second->info.state != ProcessState::Running) {
        return false;
    }

#ifdef _WIN32
    if (it->second->processHandle) {
        it->second->monitorThread.request_stop();
        const UINT uExitCode = force ? 999 : 0;
        TerminateProcess(static_cast<HANDLE>(it->second->processHandle), uExitCode);
        return true;
    }
#endif
    return false;
}

bool ProcessLauncher::isRunning(steam::core::AppId appId) const {
    std::scoped_lock lock(m_mutex);
    if (auto it = m_processes.find(appId); it != m_processes.end()) {
        return it->second->info.state == ProcessState::Running;
    }
    return false;
}

ProcessExecutionInfo ProcessLauncher::getInfo(steam::core::AppId appId) const {
    std::scoped_lock lock(m_mutex);
    if (auto it = m_processes.find(appId); it != m_processes.end()) {
        return it->second->info;
    }
    return {};
}

void ProcessLauncher::setProcessStateCallback(ProcessStateCallback callback) {
    std::scoped_lock lock(m_mutex);
    m_stateCallback = std::move(callback);
}

void ProcessLauncher::setPlaytimeCallback(PlaytimeCallback callback) {
    std::scoped_lock lock(m_mutex);
    m_playtimeCallback = std::move(callback);
}

} // namespace steam::launcher
