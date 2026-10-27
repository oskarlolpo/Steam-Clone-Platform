#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "steam/core/AppManifest.hpp"
#include "steam/core/Sha256.hpp"
#include "steam/core/UserSession.hpp"
#include "steam/downloader/DownloadManager.hpp"
#include "steam/launcher/ProcessLauncher.hpp"
#include "steam/storage/JsonStorage.hpp"
#include "steam/ui/SteamMainWindow.hpp"

namespace {

void runCliMode(
    std::shared_ptr<steam::storage::JsonStorage> storage,
    std::shared_ptr<steam::downloader::DownloadManager> downloadManager,
    std::shared_ptr<steam::launcher::ProcessLauncher> launcher
) {
    std::cout << "=========================================================\n";
    std::cout << "        STEAM CLIENT ENGINE [C++20 AUTOMATED DEMO]       \n";
    std::cout << "=========================================================\n\n";

    steam::core::UserSession session(1001, "GabeN_Fan", "token_sec_9948214fa8");
    std::cout << "[AUTH] Active User: " << session.getUsername() 
              << " (ID: " << session.getUserId() << ")\n";

    auto apps = storage->getAllApps();
    std::cout << "\n[LIBRARY] Games in library: " << apps.size() << "\n";
    for (const auto& app : apps) {
        std::cout << "  -> [" << app.id << "] " << app.title 
                  << " | Status: " << static_cast<int>(app.state)
                  << " | Playtime: " << (app.totalPlaytimeSeconds / 60) << " mins\n";
    }

    if (apps.empty()) return;

    auto targetApp = apps.front();
    const auto downloadTarget = targetApp.installDirectory / targetApp.executableRelativePath;

    std::cout << "\n[DOWNLOAD ENGINE] Initializing Chunked Multi-threaded Downloader...\n";
    downloadManager->setStatusCallback([](steam::downloader::DownloadId id, steam::downloader::DownloadStatus /*status*/, const std::string& msg) {
        std::cout << "  [Task #" << id << "] Status: " << msg << "\n";
    });

    downloadManager->setProgressCallback([](steam::downloader::DownloadId id, const steam::downloader::DownloadProgress& prog) {
        const double mbDownloaded = static_cast<double>(prog.bytesDownloaded) / (1024.0 * 1024.0);
        const double mbTotal = static_cast<double>(prog.totalBytes) / (1024.0 * 1024.0);
        const double speedMbps = (prog.currentSpeedBps * 8.0) / (1000.0 * 1000.0);

        std::cout << "\r  [Task #" << id << "] Progress: " 
                  << std::fixed << std::setprecision(1) << (prog.progressFraction * 100.0f) << "% "
                  << "(" << mbDownloaded << " / " << mbTotal << " MB) "
                  << "@ " << speedMbps << " Mbps " << std::flush;
    });

    steam::downloader::DownloadRequest request;
    request.url = targetApp.downloadUrl;
    request.destinationPath = downloadTarget;
    request.parallelChunks = 4;
    request.allowResume = true;

    const auto downloadId = downloadManager->enqueue(request);

    while (true) {
        const auto status = downloadManager->getStatus(downloadId);
        if (status == steam::downloader::DownloadStatus::Completed || 
            status == steam::downloader::DownloadStatus::Failed ||
            status == steam::downloader::DownloadStatus::Canceled) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "\n";

    const std::string fileHash = steam::core::Sha256::hashFile(downloadTarget);
    std::cout << "[INTEGRITY] Computed File SHA-256: " << fileHash << "\n";

    targetApp.state = steam::core::AppState::Installed;
    targetApp.sha256Checksum = fileHash;
    storage->saveAppManifest(targetApp);
    storage->save();

    std::cout << "\n[PROCESS LAUNCHER] Preparing game execution...\n";
    launcher->setProcessStateCallback([](steam::core::AppId id, steam::launcher::ProcessState state, int code) {
        std::cout << "  [LAUNCHER EVENT] App " << id << " state: " << static_cast<int>(state)
                  << " (ExitCode: " << code << ")\n";
    });

    launcher->setPlaytimeCallback([storage](steam::core::AppId id, std::chrono::seconds sessionDuration) {
        std::cout << "  [PLAYTIME TRACKER] App " << id << " session finished. Played: " 
                  << sessionDuration.count() << " seconds.\n";
        storage->addPlaytime(id, sessionDuration.count());
        storage->save();
    });

    steam::launcher::LaunchConfig launchConfig;
#ifdef _WIN32
    launchConfig.executablePath = "C:\\Windows\\System32\\cmd.exe";
    launchConfig.workingDirectory = std::filesystem::current_path();
    launchConfig.arguments = {"/c", "ping -n 4 127.0.0.1 >nul"};
#else
    launchConfig.executablePath = "/bin/sh";
    launchConfig.arguments = {"-c", "sleep 3"};
#endif

    const bool launched = launcher->launch(targetApp.id, launchConfig);
    if (launched) {
        const auto info = launcher->getInfo(targetApp.id);
        std::cout << "  [LAUNCHED] Game running with PID: " << info.pid << "\n";

        while (launcher->isRunning(targetApp.id)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

    auto updatedApp = storage->getAppManifest(targetApp.id);
    if (updatedApp) {
        std::cout << "\n[LIBRARY UPDATED] Total Playtime recorded: " 
                  << updatedApp->totalPlaytimeSeconds << " seconds.\n";
    }

    std::cout << "\n[SHUTDOWN] All subsystems cleanly released RAII resources.\n";
}

} // namespace

int main(int argc, char* argv[]) {
    bool cliMode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--cli") {
            cliMode = true;
        }
    }

    const std::filesystem::path storagePath = "steam_data/library.json";
    auto storage = std::make_shared<steam::storage::JsonStorage>(storagePath);
    storage->load();

    auto downloadManager = std::make_shared<steam::downloader::DownloadManager>(4);
    auto launcher = std::make_shared<steam::launcher::ProcessLauncher>();

    if (cliMode) {
        runCliMode(storage, downloadManager, launcher);
        return 0;
    }

    // Default: Launch Steam Graphical User Interface (GUI)
    steam::ui::SteamMainWindow mainWindow(storage, downloadManager, launcher);
    if (!mainWindow.create(960, 620)) {
        std::cerr << "Failed to create Steam MainWindow. Falling back to CLI mode." << std::endl;
        runCliMode(storage, downloadManager, launcher);
        return 1;
    }

    return mainWindow.runEventLoop();
}
