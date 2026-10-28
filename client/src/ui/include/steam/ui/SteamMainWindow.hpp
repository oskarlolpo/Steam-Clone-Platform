#pragma once

#include "steam/core/AppManifest.hpp"
#include "steam/core/UserSession.hpp"
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

struct StoreFeaturedGame {
    steam::core::AppId id;
    std::wstring title;
    std::wstring genre;
    std::wstring discount;
    std::wstring oldPrice;
    std::wstring newPrice;
    double priceRub;
    Gdiplus::Color themeColor;
    std::wstring description;
};

class SteamMainWindow {
public:
    SteamMainWindow(
        std::shared_ptr<steam::storage::ILocalStorage> storage,
        std::shared_ptr<steam::downloader::IDownloadManager> downloadManager,
        std::shared_ptr<steam::launcher::IProcessLauncher> processLauncher,
        steam::core::UserSession session = { 1001, "ØSKARŁØŁPØ⁴²", "token_sec_9948214fa8" }
    );
    ~SteamMainWindow();

    bool create(int width = 1180, int height = 740);
    int runEventLoop();

    static std::wstring utf8ToWide(std::string_view utf8);
    static std::string wideToUtf8(std::wstring_view wide);

private:
#ifdef _WIN32
    static LRESULT CALLBACK windowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void onPaint(HWND hwnd);
    void onClick(int x, int y);
    void onMouseMove(int x, int y);
    void onMouseLeave();
    void onTimer();
    void onChar(wchar_t ch);
    void onKeyDown(WPARAM key);

    // Tab rendering functions
    void drawTopNav(Gdiplus::Graphics& g, const RECT& r);
    void drawStoreTab(Gdiplus::Graphics& g, const RECT& r);
    void drawLibraryTab(Gdiplus::Graphics& g, const RECT& r);
    void drawCommunityTab(Gdiplus::Graphics& g, const RECT& r);
    void drawBottomBar(Gdiplus::Graphics& g, const RECT& r);
    void drawToast(Gdiplus::Graphics& g, const RECT& r);

    // Helpers
    void drawWallpaper(Gdiplus::Graphics& g, int x, int y, int w, int h, const steam::core::AppManifest& app);
    void ensureGamesInitialized();
    void updateFilteredApps();
    void onAddNonSteamGame();
    void onBuyOrAddStoreGame(const StoreFeaturedGame& game);
    void showToast(std::wstring message);

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
    steam::core::UserSession m_session;
    double m_walletBalance{75.31};

    MainTab m_currentTab{MainTab::Library};
    std::vector<steam::core::AppManifest> m_apps;
    std::vector<std::size_t> m_filteredIndices;
    std::size_t m_selectedIndex{0};

    // Library Search State
    std::wstring m_searchQuery;
    bool m_isSearchActive{false};
    bool m_cursorBlink{false};

    // Store State
    int m_storeCarouselIndex{0};
    std::vector<StoreFeaturedGame> m_storeGames;
    std::vector<StoreFeaturedGame> m_storeDiscounts;

    // Hover Tracking
    int m_hoveredSidebarIndex{-1};
    int m_hoveredStoreCard{-1};
    bool m_hoveredPlayBtn{false};
    bool m_hoveredAddGame{false};
    bool m_hoveredSearch{false};
    bool m_mouseTracking{false};

    // Toast Notification
    std::wstring m_toastMessage;
    int m_toastTimer{0};

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
