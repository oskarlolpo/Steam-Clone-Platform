#pragma once

#include "steam/core/AppManifest.hpp"
#include "steam/downloader/IDownloadManager.hpp"
#include "steam/launcher/IProcessLauncher.hpp"
#include "steam/storage/ILocalStorage.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#endif

namespace steam::ui {

enum class MainTab {
    Store,
    Library,
    Community
};

class SteamMainWindow {
public:
    SteamMainWindow(
        std::shared_ptr<steam::storage::ILocalStorage> storage,
        std::shared_ptr<steam::downloader::IDownloadManager> downloadManager,
        std::shared_ptr<steam::launcher::IProcessLauncher> processLauncher
    );
    ~SteamMainWindow();

    bool create(int width = 1100, int height = 720);
    int runEventLoop();

    static std::wstring utf8ToWide(std::string_view utf8);

private:
#ifdef _WIN32
    static LRESULT CALLBACK windowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onPaint(HWND hwnd);
    void onClick(int x, int y);
    void onTimer();

    // Tab rendering functions
    void drawTopNav(Gdiplus::Graphics& g, const RECT& r);
    void drawStoreTab(Gdiplus::Graphics& g, const RECT& r);
    void drawLibraryTab(Gdiplus::Graphics& g, const RECT& r);
    void drawCommunityTab(Gdiplus::Graphics& g, const RECT& r);
    void drawBottomBar(Gdiplus::Graphics& g, const RECT& r);

    // Helpers
    void drawWallpaper(Gdiplus::Graphics& g, int x, int y, int w, int h, const steam::core::AppManifest& app);
    void ensureGamesInitialized();

    HWND m_hwnd{nullptr};
    ULONG_PTR m_gdiplusToken{0};

    HFONT m_fontMenu{nullptr};
    HFONT m_fontTabs{nullptr};
    HFONT m_fontTitle{nullptr};
    HFONT m_fontSubTitle{nullptr};
    HFONT m_fontNormal{nullptr};
    HFONT m_fontBold{nullptr};
    HFONT m_fontSmall{nullptr};
#endif

    std::shared_ptr<steam::storage::ILocalStorage> m_storage;
    std::shared_ptr<steam::downloader::IDownloadManager> m_downloadManager;
    std::shared_ptr<steam::launcher::IProcessLauncher> m_processLauncher;

    MainTab m_currentTab{MainTab::Library};
    std::vector<steam::core::AppManifest> m_apps;
    std::size_t m_selectedIndex{0};

    // Store state
    int m_storeCarouselIndex{1}; // 0 = ATS, 1 = Ready or Not, 2 = Bodycam
    int m_storeSubIndex{0};

    // Live state tracking
    bool m_isDownloading{false};
    float m_downloadProgressFraction{0.0f};
    double m_downloadSpeedMbps{0.0};
    std::uint64_t m_downloadActiveId{0};
    std::string m_statusMessage{"Готово к игре"};

    bool m_isGameRunning{false};
    std::uint32_t m_runningPid{0};
    std::uint64_t m_currentSessionSeconds{0};
    steam::core::AppId m_runningAppId{0};
};

} // namespace steam::ui
