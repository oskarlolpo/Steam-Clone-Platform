#include "steam/ui/SteamMainWindow.hpp"
#include <algorithm>
#include <chrono>
#include <commdlg.h>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#ifndef WM_APP_DOWNLOAD_PROGRESS
#define WM_APP_DOWNLOAD_PROGRESS (WM_APP + 101)
#define WM_APP_DOWNLOAD_STATUS   (WM_APP + 102)
#define WM_APP_GAME_STATE        (WM_APP + 103)
#endif

namespace steam::ui {

std::wstring SteamMainWindow::utf8ToWide(std::string_view utf8) {
    if (utf8.empty()) return {};
    const int count = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    if (count <= 0) return {};
    std::wstring result(static_cast<std::size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), result.data(), count);
    return result;
}

std::string SteamMainWindow::wideToUtf8(std::wstring_view wide) {
    if (wide.empty()) return {};
    const int count = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (count <= 0) return {};
    std::string result(static_cast<std::size_t>(count), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), result.data(), count, nullptr, nullptr);
    return result;
}

SteamMainWindow::SteamMainWindow(
    std::shared_ptr<steam::storage::ILocalStorage> storage,
    std::shared_ptr<steam::downloader::IDownloadManager> downloadManager,
    std::shared_ptr<steam::launcher::IProcessLauncher> processLauncher,
    steam::core::UserSession session
) : m_storage(std::move(storage)),
    m_downloadManager(std::move(downloadManager)),
    m_processLauncher(std::move(processLauncher)),
    m_session(std::move(session))
{
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    // Initialize Store Catalog
    m_storeGames = {
        {
            5001,
            L"AMERICAN TRUCK SIMULATOR",
            L"Симулятор вождения / Открытый мир",
            L"-75%",
            L"1249 руб.",
            L"312 руб.",
            312.0,
            Gdiplus::Color(255, 35, 65, 105),
            L"Легендарный симулятор дальнобойщика по дорогам Северной Америки с реалистичной физикой."
        },
        {
            5002,
            L"READY OR NOT",
            L"Тактический шутер / Спецназ SWAT",
            L"-50%",
            L"2240 руб.",
            L"1120 руб.",
            1120.0,
            Gdiplus::Color(255, 130, 35, 35),
            L"Бескомпромиссный тактический симулятор спецназа в условиях повышенной опасности."
        },
        {
            5003,
            L"BODYCAM",
            L"Фотореалистичный FPS / Мультиплеер",
            L"-20%",
            L"1200 руб.",
            L"960 руб.",
            960.0,
            Gdiplus::Color(255, 50, 60, 75),
            L"Сверхреалистичный мультиплеерный шутер от первого лица с эффектом нательной камеры."
        }
    };

    m_storeDiscounts = {
        {
            6001,
            L"Warhammer 40,000: Mechanicus",
            L"Пошаговая тактика",
            L"-95%",
            L"1100 руб.",
            L"55 руб.",
            55.0,
            Gdiplus::Color(255, 30, 55, 45),
            L"Управляйте техножрецами Адептус Механикус на неизведанной планете Сильва Тенебрис."
        },
        {
            6002,
            L"Jagged Alliance 3",
            L"Тактическая RPG",
            L"-90%",
            L"1999 руб.",
            L"199 руб.",
            199.0,
            Gdiplus::Color(255, 70, 50, 32),
            L"Нанимайте уникальных наёмников и освободите страну Гранд Чиен в пошаговых боях."
        },
        {
            6003,
            L"PlateUp!",
            L"Кооперативный рогалик",
            L"-90%",
            L"650 руб.",
            L"65 руб.",
            65.0,
            Gdiplus::Color(255, 60, 45, 75),
            L"Готовьте блюда, обслуживайте клиентов и стройте ресторанную империю вместе с друзьями."
        }
    };

    ensureGamesInitialized();
    updateFilteredApps();
}

SteamMainWindow::~SteamMainWindow() {
    if (m_fontMenu) DeleteObject(m_fontMenu);
    if (m_fontTabs) DeleteObject(m_fontTabs);
    if (m_fontTitle) DeleteObject(m_fontTitle);
    if (m_fontSubTitle) DeleteObject(m_fontSubTitle);
    if (m_fontNormal) DeleteObject(m_fontNormal);
    if (m_fontBold) DeleteObject(m_fontBold);
    if (m_fontSmall) DeleteObject(m_fontSmall);

    if (m_gdiplusToken) {
        Gdiplus::GdiplusShutdown(m_gdiplusToken);
    }
}

void SteamMainWindow::ensureGamesInitialized() {
    m_apps = m_storage->getAllApps();

    // 1. Ensure D&D game
    bool hasDnd = false;
    for (auto& app : m_apps) {
        if (app.id == 101 || app.title.find("Dungeons & Dragons") != std::string::npos) {
            hasDnd = true;
            app.id = 101;
            app.title = "Dungeons & Dragons: Console RPG";
            app.version = "1.0.0 (Release)";
            app.installDirectory = "games/Console-DnD";
            app.executableRelativePath = "game.exe";
            app.state = steam::core::AppState::Installed;
            m_storage->saveAppManifest(app);
            break;
        }
    }

    if (!hasDnd) {
        steam::core::AppManifest dndGame;
        dndGame.id = 101;
        dndGame.title = "Dungeons & Dragons: Console RPG";
        dndGame.version = "1.0.0 (Release)";
        dndGame.installSize = 734720;
        dndGame.installDirectory = "games/Console-DnD";
        dndGame.executableRelativePath = "game.exe";
        dndGame.downloadUrl = "https://cdn.steam-clone.local/depot/dnd_rpg.bin";
        dndGame.state = steam::core::AppState::Installed;
        dndGame.totalPlaytimeSeconds = 2520;
        m_storage->saveAppManifest(dndGame);
    }

    // 2. Ensure CS2 Demake / Tactical 3D FPS
    bool hasCs2 = false;
    for (auto& app : m_apps) {
        if (app.id == 570 || app.title.find("Counter-Strike") != std::string::npos || app.title.find("Tactical Strike") != std::string::npos) {
            hasCs2 = true;
            app.id = 570;
            app.title = "Counter-Strike 2: Source Edition";
            app.version = "2.1.0";
            app.installDirectory = "steamapps/common/CS2Source";
            app.executableRelativePath = "cs2.exe";
            app.state = steam::core::AppState::Installed;
            m_storage->saveAppManifest(app);
            break;
        }
    }

    if (!hasCs2) {
        steam::core::AppManifest cs2;
        cs2.id = 570;
        cs2.title = "Counter-Strike 2: Source Edition";
        cs2.version = "2.1.0";
        cs2.installSize = 25 * 1024 * 1024;
        cs2.installDirectory = "steamapps/common/CS2Source";
        cs2.executableRelativePath = "cs2.exe";
        cs2.downloadUrl = "https://cdn.steam-clone.local/depot/cs2.bin";
        cs2.state = steam::core::AppState::Installed;
        cs2.totalPlaytimeSeconds = 14208;
        m_storage->saveAppManifest(cs2);
    }

    // 3. Ensure HL3 & Portal
    bool hasHl3 = false;
    bool hasPortal = false;
    for (const auto& a : m_apps) {
        if (a.id == 730) hasHl3 = true;
        if (a.id == 400) hasPortal = true;
    }

    if (!hasHl3) {
        steam::core::AppManifest hl3;
        hl3.id = 730;
        hl3.title = "Half-Life 3 (Preview Build)";
        hl3.version = "0.9.4-alpha";
        hl3.installSize = 10 * 1024 * 1024;
        hl3.installDirectory = "steamapps/common/HalfLife3";
        hl3.executableRelativePath = "hl3_game.exe";
        hl3.downloadUrl = "https://cdn.steam-clone.local/depot/hl3.bin";
        hl3.state = steam::core::AppState::Installed;
        hl3.totalPlaytimeSeconds = 3600;
        m_storage->saveAppManifest(hl3);
    }

    if (!hasPortal) {
        steam::core::AppManifest portal;
        portal.id = 400;
        portal.title = "Portal: Revolution Extended";
        portal.version = "1.4.2";
        portal.installSize = 8 * 1024 * 1024;
        portal.installDirectory = "steamapps/common/PortalRev";
        portal.executableRelativePath = "portal_rev.exe";
        portal.downloadUrl = "https://cdn.steam-clone.local/depot/portal.bin";
        portal.state = steam::core::AppState::Installed;
        portal.totalPlaytimeSeconds = 6200;
        m_storage->saveAppManifest(portal);
    }

    m_storage->save();
    m_apps = m_storage->getAllApps();
}

void SteamMainWindow::updateFilteredApps() {
    m_filteredIndices.clear();
    if (m_searchQuery.empty()) {
        m_filteredIndices.reserve(m_apps.size());
        for (std::size_t i = 0; i < m_apps.size(); ++i) {
            m_filteredIndices.push_back(i);
        }
        return;
    }

    std::wstring lowerQuery = m_searchQuery;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::towlower);

    for (std::size_t i = 0; i < m_apps.size(); ++i) {
        std::wstring titleW = utf8ToWide(m_apps[i].title);
        std::wstring lowerTitle = titleW;
        std::transform(lowerTitle.begin(), lowerTitle.end(), lowerTitle.begin(), ::towlower);

        if (lowerTitle.find(lowerQuery) != std::wstring::npos) {
            m_filteredIndices.push_back(i);
        }
    }

    // Ensure selected index is valid
    if (!m_filteredIndices.empty()) {
        bool selectedInFilter = false;
        for (auto idx : m_filteredIndices) {
            if (idx == m_selectedIndex) {
                selectedInFilter = true;
                break;
            }
        }
        if (!selectedInFilter) {
            m_selectedIndex = m_filteredIndices.front();
        }
    }
}

bool SteamMainWindow::create(int width, int height) {
    const wchar_t CLASS_NAME[] = L"SteamClientModernWindowClass";

    WNDCLASSW wc{};
    wc.lpfnWndProc = SteamMainWindow::windowProcStatic;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(23, 26, 33));

    RegisterClassW(&wc);

    const int screenW = GetSystemMetrics(SM_CXSCREEN);
    const int screenH = GetSystemMetrics(SM_CYSCREEN);
    const int posX = (screenW - width) / 2;
    const int posY = (screenH - height) / 2;

    m_hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Steam",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX,
        posX, posY, width, height,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!m_hwnd) {
        return false;
    }

    SetWindowTextW(m_hwnd, L"Steam");

    // ClearType Fonts
    m_fontMenu = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    m_fontTabs = CreateFontW(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    m_fontTitle = CreateFontW(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    m_fontSubTitle = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    m_fontNormal = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    m_fontBold = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    m_fontSmall = CreateFontW(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

    // Setup Engine Callbacks
    m_downloadManager->setProgressCallback([this](steam::downloader::DownloadId, const steam::downloader::DownloadProgress& prog) {
        if (m_hwnd) {
            PostMessage(m_hwnd, WM_APP_DOWNLOAD_PROGRESS, static_cast<WPARAM>(prog.progressFraction * 1000.0f), static_cast<LPARAM>(prog.currentSpeedBps / 1024.0));
        }
    });

    m_downloadManager->setStatusCallback([this](steam::downloader::DownloadId, steam::downloader::DownloadStatus status, const std::string&) {
        if (m_hwnd) {
            PostMessage(m_hwnd, WM_APP_DOWNLOAD_STATUS, static_cast<WPARAM>(status), 0);
        }
    });

    m_processLauncher->setProcessStateCallback([this](steam::core::AppId, steam::launcher::ProcessState state, int code) {
        if (m_hwnd) {
            PostMessage(m_hwnd, WM_APP_GAME_STATE, static_cast<WPARAM>(state), static_cast<LPARAM>(code));
        }
    });

    m_processLauncher->setPlaytimeCallback([this](steam::core::AppId appId, std::chrono::seconds duration) {
        m_storage->addPlaytime(appId, duration.count());
        m_storage->save();
    });

    // 500ms Timer: handles cursor blink, session clock, and toast notification fade
    SetTimer(m_hwnd, 1, 500, nullptr);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    return true;
}

LRESULT CALLBACK SteamMainWindow::windowProcStatic(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SteamMainWindow* pThis = nullptr;
    if (uMsg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<SteamMainWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<SteamMainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->windowProc(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT SteamMainWindow::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            onPaint(hwnd);
            return 0;

        case WM_MOUSEMOVE: {
            if (!m_mouseTracking) {
                TRACKMOUSEEVENT tme{ sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0 };
                TrackMouseEvent(&tme);
                m_mouseTracking = true;
            }
            const int x = LOWORD(lParam);
            const int y = HIWORD(lParam);
            onMouseMove(x, y);
            return 0;
        }

        case WM_MOUSELEAVE:
            m_mouseTracking = false;
            onMouseLeave();
            return 0;

        case WM_LBUTTONDOWN: {
            const int x = LOWORD(lParam);
            const int y = HIWORD(lParam);
            onClick(x, y);
            return 0;
        }

        case WM_CHAR:
            onChar(static_cast<wchar_t>(wParam));
            return 0;

        case WM_KEYDOWN:
            onKeyDown(wParam);
            return 0;

        case WM_TIMER:
            onTimer();
            return 0;

        case WM_APP_DOWNLOAD_PROGRESS: {
            m_downloadProgressFraction = static_cast<float>(wParam) / 1000.0f;
            m_downloadSpeedMbps = (static_cast<double>(lParam) * 1024.0 * 8.0) / 1'000'000.0;
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_APP_DOWNLOAD_STATUS: {
            auto status = static_cast<steam::downloader::DownloadStatus>(wParam);
            if (status == steam::downloader::DownloadStatus::Completed) {
                m_isDownloading = false;
                m_statusMessage = "Загрузка завершена! Целостность подтверждена.";
                showToast(L"Загрузка завершена!");
                if (m_selectedIndex < m_apps.size()) {
                    m_apps[m_selectedIndex].state = steam::core::AppState::Installed;
                    m_storage->saveAppManifest(m_apps[m_selectedIndex]);
                    m_storage->save();
                }
            } else if (status == steam::downloader::DownloadStatus::Downloading) {
                m_isDownloading = true;
                m_statusMessage = "Скачивание файлов игры...";
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_APP_GAME_STATE: {
            auto state = static_cast<steam::launcher::ProcessState>(wParam);
            if (state == steam::launcher::ProcessState::Running) {
                m_isGameRunning = true;
                m_currentSessionSeconds = 0;
                m_statusMessage = "Игра запущена";
                showToast(L"Игра запущена!");
            } else {
                m_isGameRunning = false;
                m_statusMessage = "Игра закрыта";
                showToast(L"Игра завершена");
                m_apps = m_storage->getAllApps();
                updateFilteredApps();
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SteamMainWindow::onTimer() {
    m_cursorBlink = !m_cursorBlink;

    if (m_isGameRunning) {
        m_currentSessionSeconds++;
    }

    if (m_toastTimer > 0) {
        m_toastTimer--;
    }

    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void SteamMainWindow::showToast(std::wstring message) {
    m_toastMessage = std::move(message);
    m_toastTimer = 8; // ~4 seconds
    if (m_hwnd) InvalidateRect(m_hwnd, nullptr, FALSE);
}

void SteamMainWindow::onMouseMove(int x, int y) {
    RECT r;
    GetClientRect(m_hwnd, &r);

    bool changed = false;

    // Check search box hover
    bool wasHoverSearch = m_hoveredSearch;
    m_hoveredSearch = (m_currentTab == MainTab::Library && x >= 12 && x <= 258 && y >= 72 && y <= 98);
    if (wasHoverSearch != m_hoveredSearch) changed = true;

    // Check add game hover
    bool wasHoverAdd = m_hoveredAddGame;
    m_hoveredAddGame = (x >= 12 && x <= 160 && y >= r.bottom - 28 && y <= r.bottom - 4);
    if (wasHoverAdd != m_hoveredAddGame) changed = true;

    // Check play button hover
    bool wasHoverPlay = m_hoveredPlayBtn;
    m_hoveredPlayBtn = (m_currentTab == MainTab::Library && x >= 300 && x <= 500 && y >= 195 && y <= 245);
    if (wasHoverPlay != m_hoveredPlayBtn) changed = true;

    // Check sidebar item hover
    int oldSidebar = m_hoveredSidebarIndex;
    m_hoveredSidebarIndex = -1;
    if (m_currentTab == MainTab::Library && x < 270 && y >= 130) {
        int idx = (y - 130) / 46;
        if (idx >= 0 && idx < static_cast<int>(m_filteredIndices.size())) {
            m_hoveredSidebarIndex = idx;
        }
    }
    if (oldSidebar != m_hoveredSidebarIndex) changed = true;

    if (changed) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void SteamMainWindow::onMouseLeave() {
    m_hoveredSidebarIndex = -1;
    m_hoveredStoreCard = -1;
    m_hoveredPlayBtn = false;
    m_hoveredAddGame = false;
    m_hoveredSearch = false;
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void SteamMainWindow::onChar(wchar_t ch) {
    if (!m_isSearchActive) return;

    if (ch == VK_BACK) {
        if (!m_searchQuery.empty()) {
            m_searchQuery.pop_back();
            updateFilteredApps();
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
        return;
    }

    if (ch >= 32) {
        m_searchQuery.push_back(ch);
        updateFilteredApps();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void SteamMainWindow::onKeyDown(WPARAM key) {
    if (key == VK_ESCAPE) {
        if (m_isSearchActive) {
            m_isSearchActive = false;
            m_searchQuery.clear();
            updateFilteredApps();
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
    }
}

void SteamMainWindow::onAddNonSteamGame() {
    wchar_t szFile[MAX_PATH] = L"";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hwnd;
    ofn.lpstrFilter = L"Исполняемые файлы (*.exe)\0*.exe\0Все файлы (*.*)\0*.*\0";
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrTitle = L"Выберите исполняемый файл игры (.exe)";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn)) {
        std::filesystem::path p(szFile);
        std::wstring gameName = p.stem().wstring();

        // Generate unique AppId
        steam::core::AppId newId = 9000;
        for (const auto& a : m_apps) {
            if (a.id >= newId) newId = a.id + 1;
        }

        steam::core::AppManifest newApp;
        newApp.id = newId;
        newApp.title = wideToUtf8(gameName);
        newApp.version = "1.0.0 (Custom)";
        newApp.installDirectory = p.parent_path().string();
        newApp.executableRelativePath = p.filename().string();
        newApp.state = steam::core::AppState::Installed;
        try {
            newApp.installSize = std::filesystem::file_size(p);
        } catch (...) {
            newApp.installSize = 10 * 1024 * 1024;
        }
        newApp.totalPlaytimeSeconds = 0;

        m_storage->saveAppManifest(newApp);
        m_storage->save();

        m_apps = m_storage->getAllApps();
        updateFilteredApps();

        // Select the newly added game
        for (std::size_t i = 0; i < m_apps.size(); ++i) {
            if (m_apps[i].id == newId) {
                m_selectedIndex = i;
                break;
            }
        }

        m_currentTab = MainTab::Library;
        showToast(L"Игра «" + gameName + L"» добавлена в библиотеку!");
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void SteamMainWindow::onBuyOrAddStoreGame(const StoreFeaturedGame& game) {
    // Check if already in library
    for (std::size_t i = 0; i < m_apps.size(); ++i) {
        if (m_apps[i].id == game.id) {
            m_selectedIndex = i;
            m_currentTab = MainTab::Library;
            showToast(L"«" + game.title + L"» уже в вашей библиотеке!");
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }
    }

    // Add new game to library
    steam::core::AppManifest newApp;
    newApp.id = game.id;
    newApp.title = wideToUtf8(game.title);
    newApp.version = "1.0.0";
    newApp.installSize = 15ULL * 1024ULL * 1024ULL * 1024ULL;
    newApp.installDirectory = "steamapps/common/" + wideToUtf8(game.title);
    newApp.executableRelativePath = "game.exe";
    newApp.state = steam::core::AppState::Installed; // Marked as ready
    newApp.totalPlaytimeSeconds = 0;

    m_storage->saveAppManifest(newApp);
    m_storage->save();

    m_apps = m_storage->getAllApps();
    updateFilteredApps();

    for (std::size_t i = 0; i < m_apps.size(); ++i) {
        if (m_apps[i].id == game.id) {
            m_selectedIndex = i;
            break;
        }
    }

    if (m_walletBalance >= game.priceRub) {
        m_walletBalance -= game.priceRub;
    }

    m_currentTab = MainTab::Library;
    showToast(L"Куплено и добавлено в библиотеку: " + game.title);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void SteamMainWindow::onClick(int x, int y) {
    RECT r;
    GetClientRect(m_hwnd, &r);

    // 1. Top Navigation Tabs
    if (y >= 24 && y <= 58) {
        if (x >= 65 && x <= 165) {
            m_currentTab = MainTab::Store;
            m_isSearchActive = false;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        } else if (x >= 170 && x <= 295) {
            m_currentTab = MainTab::Library;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        } else if (x >= 300 && x <= 430) {
            m_currentTab = MainTab::Community;
            m_isSearchActive = false;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }
    }

    // 2. Bottom Bar: «➕ Добавить игру»
    if (y >= r.bottom - 30 && y <= r.bottom - 2 && x >= 12 && x <= 180) {
        onAddNonSteamGame();
        return;
    }

    // 3. Tab Specific Interactions
    if (m_currentTab == MainTab::Library) {
        // Search Box Click (x: 12..258, y: 72..98)
        if (x >= 12 && x <= 258 && y >= 72 && y <= 98) {
            m_isSearchActive = true;
            // Clear icon click on right side of search bar
            if (x >= 235 && !m_searchQuery.empty()) {
                m_searchQuery.clear();
                updateFilteredApps();
            }
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        } else {
            m_isSearchActive = false;
        }

        // Sidebar Item Click (x < 270, y >= 130)
        constexpr int sidebarWidth = 270;
        constexpr int listStartY = 130;
        constexpr int itemH = 46;

        if (x < sidebarWidth && y >= listStartY) {
            const std::size_t filteredIdx = static_cast<std::size_t>((y - listStartY) / itemH);
            if (filteredIdx < m_filteredIndices.size()) {
                m_selectedIndex = m_filteredIndices[filteredIdx];
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return;
            }
        }

        // Action Button: [ ИГРАТЬ ] / [ УСТАНОВИТЬ ] / [ ОСТАНОВИТЬ ]
        const RECT btnRect{ 300, 195, 500, 245 };
        if (x >= btnRect.left && x <= btnRect.right && y >= btnRect.top && y <= btnRect.bottom) {
            if (m_selectedIndex >= m_apps.size()) return;
            auto& app = m_apps[m_selectedIndex];

            if (m_isGameRunning) {
                m_processLauncher->terminate(app.id, true);
                showToast(L"Остановка процесса...");
            } else if (app.state == steam::core::AppState::Installed) {
                // Launch game with robust relative-path resolution
                steam::launcher::LaunchConfig config;
                config.createNewConsole = false;

                std::filesystem::path targetExe;
                std::filesystem::path workingDir;

                // Candidate searches
                if (app.id == 570) {
                    // Counter-Strike 2 / Tactical FPS
                    std::vector<std::filesystem::path> candidates = {
                        std::filesystem::current_path() / "steamapps" / "common" / "CS2Source" / "cs2.exe",
                        std::filesystem::current_path() / "steamapps" / "common" / "CS2Source" / "CS2WinFormsDemo.exe",
                        std::filesystem::current_path() / "games" / "CS2WinFormsDemo" / "bin" / "Release" / "publish" / "CS2WinFormsDemo.exe",
                        std::filesystem::current_path() / ".." / "games" / "CS2WinFormsDemo" / "bin" / "Release" / "publish" / "CS2WinFormsDemo.exe",
                        std::filesystem::current_path() / ".." / "CS2WinFormsDemo" / "bin" / "Release" / "publish" / "CS2WinFormsDemo.exe"
                    };
                    for (const auto& c : candidates) {
                        if (std::filesystem::exists(c)) {
                            targetExe = c;
                            workingDir = c.parent_path();
                            break;
                        }
                    }
                } else if (app.id == 101) {
                    // D&D Console RPG
                    std::vector<std::filesystem::path> candidates = {
                        std::filesystem::current_path() / "games" / "Console-DnD" / "game.exe",
                        std::filesystem::current_path() / ".." / "games" / "Console-DnD" / "game.exe",
                        std::filesystem::current_path() / ".." / "ind" / "Console-DnD-main" / "game.exe",
                        std::filesystem::path("C:\\Users\\govno\\Desktop\\zadaniy\\ind\\Console-DnD-main\\game.exe")
                    };
                    for (const auto& c : candidates) {
                        if (std::filesystem::exists(c)) {
                            targetExe = c;
                            workingDir = c.parent_path();
                            config.createNewConsole = true;
                            break;
                        }
                    }
                } else {
                    // Custom / Installed Games
                    std::filesystem::path p1 = app.installDirectory / app.executableRelativePath;
                    std::filesystem::path p2 = std::filesystem::current_path() / app.installDirectory / app.executableRelativePath;
                    if (std::filesystem::exists(p1)) {
                        targetExe = p1;
                        workingDir = p1.parent_path();
                    } else if (std::filesystem::exists(p2)) {
                        targetExe = p2;
                        workingDir = p2.parent_path();
                    }
                }

                if (!targetExe.empty() && std::filesystem::exists(targetExe)) {
                    config.executablePath = targetExe;
                    config.workingDirectory = workingDir;

                    if (m_processLauncher->launch(app.id, config)) {
                        const auto info = m_processLauncher->getInfo(app.id);
                        m_runningPid = info.pid;
                        m_runningAppId = app.id;
                        m_isGameRunning = true;
                        m_currentSessionSeconds = 0;
                        showToast(L"Запуск: " + utf8ToWide(app.title));
                    }
                } else {
                    // Executable not found fallback simulation
                    config.executablePath = "C:\\Windows\\System32\\cmd.exe";
                    config.workingDirectory = std::filesystem::current_path();
                    config.arguments = {"/c", "echo Steam Game Launched! & ping -n 60 127.0.0.1 >nul"};
                    if (m_processLauncher->launch(app.id, config)) {
                        m_isGameRunning = true;
                        m_runningAppId = app.id;
                        showToast(L"Запущен демо-режим игры");
                    }
                }
            } else if (!m_isDownloading) {
                // Trigger download
                steam::downloader::DownloadRequest req;
                req.url = app.downloadUrl;
                req.destinationPath = app.installDirectory / app.executableRelativePath;
                req.parallelChunks = 4;
                req.allowResume = true;
                m_downloadActiveId = m_downloadManager->enqueue(req);
                m_isDownloading = true;
                m_downloadProgressFraction = 0.0f;
                m_statusMessage = "Загрузка...";
                showToast(L"Началась загрузка: " + utf8ToWide(app.title));
            }
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }
    } else if (m_currentTab == MainTab::Store) {
        // Dynamic Carousel Dimensions
        constexpr int cardW = 340;
        constexpr int cardH = 240;
        const int totalCardsW = cardW * 3 + 40;
        const int startX = std::max<int>(20, static_cast<int>(r.right - totalCardsW) / 2);

        // Check Carousel Left Arrow
        if (x >= startX - 45 && x <= startX - 10 && y >= 190 && y <= 260) {
            m_storeCarouselIndex = (m_storeCarouselIndex + static_cast<int>(m_storeGames.size()) - 1) % static_cast<int>(m_storeGames.size());
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }

        // Check Carousel Right Arrow
        const int rightArrowX = startX + totalCardsW + 10;
        if (x >= rightArrowX && x <= rightArrowX + 40 && y >= 190 && y <= 260) {
            m_storeCarouselIndex = (m_storeCarouselIndex + 1) % static_cast<int>(m_storeGames.size());
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }

        // Check Clicks on Featured Carousel Cards (В КОРЗИНУ / КУПИТЬ)
        int curX = startX;
        for (std::size_t i = 0; i < m_storeGames.size(); ++i) {
            if (x >= curX && x <= curX + cardW && y >= 120 && y <= 120 + cardH) {
                onBuyOrAddStoreGame(m_storeGames[i]);
                return;
            }
            curX += cardW + 20;
        }

        // Check Clicks on Discount Cards (Section 2)
        int sec2Y = 385;
        curX = startX;
        for (std::size_t i = 0; i < m_storeDiscounts.size(); ++i) {
            if (x >= curX && x <= curX + cardW && y >= sec2Y + 30 && y <= sec2Y + 170) {
                onBuyOrAddStoreGame(m_storeDiscounts[i]);
                return;
            }
            curX += cardW + 20;
        }
    }
}

void SteamMainWindow::onPaint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT r;
    GetClientRect(hwnd, &r);

    // Double buffer
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, r.right, r.bottom);
    HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

    Gdiplus::Graphics g(memDC);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    // 1. Base Dark Background
    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 23, 26, 33));
    g.FillRectangle(&bgBrush, 0, 0, r.right, r.bottom);

    // 2. Top Nav
    drawTopNav(g, r);

    // 3. Active Tab
    if (m_currentTab == MainTab::Store) {
        drawStoreTab(g, r);
    } else if (m_currentTab == MainTab::Library) {
        drawLibraryTab(g, r);
    } else {
        drawCommunityTab(g, r);
    }

    // 4. Bottom status bar
    drawBottomBar(g, r);

    // 5. Toast Notification
    drawToast(g, r);

    // Blit to screen
    BitBlt(hdc, 0, 0, r.right, r.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}

void SteamMainWindow::drawTopNav(Gdiplus::Graphics& g, const RECT& r) {
    // Menu bar (Steam, Вид, Друзья, Игры, Справка)
    Gdiplus::SolidBrush menuBg(Gdiplus::Color(255, 20, 23, 29));
    g.FillRectangle(&menuBg, 0, 0, r.right, 24);

    Gdiplus::Font menuFont(L"Segoe UI", 8.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush menuTextBrush(Gdiplus::Color(255, 175, 185, 195));
    g.DrawString(L"Steam   Вид   Друзья   Игры   Справка", -1, &menuFont, Gdiplus::PointF(16, 4), &menuTextBrush);

    // Subnav Bar (#171a21)
    Gdiplus::SolidBrush subnavBg(Gdiplus::Color(255, 23, 26, 33));
    g.FillRectangle(&subnavBg, 0, 24, r.right, 40);

    // Navigation Arrows: ◀ ▶
    Gdiplus::Font arrowFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush arrowBrush(Gdiplus::Color(255, 130, 140, 150));
    g.DrawString(L"◀   ▶", -1, &arrowFont, Gdiplus::PointF(16, 30), &arrowBrush);

    // Big Tabs: МАГАЗИН, БИБЛИОТЕКА, СООБЩЕСТВО
    Gdiplus::Font tabFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);

    auto getTabBrush = [&](MainTab tab) -> Gdiplus::SolidBrush {
        if (m_currentTab == tab) {
            return Gdiplus::SolidBrush(Gdiplus::Color(255, 26, 159, 255)); // Active bright Steam Blue
        }
        return Gdiplus::SolidBrush(Gdiplus::Color(255, 215, 225, 235)); // Inactive
    };

    auto bStore = getTabBrush(MainTab::Store);
    auto bLib = getTabBrush(MainTab::Library);
    auto bComm = getTabBrush(MainTab::Community);

    g.DrawString(L"МАГАЗИН", -1, &tabFont, Gdiplus::PointF(70, 31), &bStore);
    g.DrawString(L"БИБЛИОТЕКА", -1, &tabFont, Gdiplus::PointF(180, 31), &bLib);
    g.DrawString(L"СООБЩЕСТВО", -1, &tabFont, Gdiplus::PointF(320, 31), &bComm);

    // Active Tab Underline
    Gdiplus::Pen activePen(Gdiplus::Color(255, 26, 159, 255), 3.0f);
    if (m_currentTab == MainTab::Store) {
        g.DrawLine(&activePen, 70, 56, 148, 56);
    } else if (m_currentTab == MainTab::Library) {
        g.DrawLine(&activePen, 180, 56, 288, 56);
    } else if (m_currentTab == MainTab::Community) {
        g.DrawLine(&activePen, 320, 56, 435, 56);
    }

    // Right Side: User Profile & Wallet Balance
    std::wstring userW = utf8ToWide(m_session.getUsername());
    if (userW.empty()) userW = L"ØSKARŁØŁPØ⁴²";

    std::wostringstream balSs;
    balSs << std::fixed << std::setprecision(2) << m_walletBalance << L" руб.";
    std::wstring profileText = userW + L"  •  " + balSs.str();

    // Avatar Circle
    float profileX = static_cast<float>(r.right - 310);
    Gdiplus::SolidBrush avatarBg(Gdiplus::Color(255, 26, 159, 255));
    g.FillEllipse(&avatarBg, profileX - 32.0f, 29.0f, 24.0f, 24.0f);

    Gdiplus::Font avFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush avTxt(Gdiplus::Color(255, 255, 255, 255));
    g.DrawString(L"Ø", -1, &avFont, Gdiplus::PointF(profileX - 26, 31), &avTxt);

    Gdiplus::SolidBrush userBrush(Gdiplus::Color(255, 164, 208, 7)); // Green badge
    g.DrawString(profileText.c_str(), -1, &menuFont, Gdiplus::PointF(profileX, 33), &userBrush);
}

void SteamMainWindow::drawWallpaper(Gdiplus::Graphics& g, int x, int y, int w, int h, const steam::core::AppManifest& app) {
    Gdiplus::Rect bannerRect(x, y, w, h);

    if (app.id == 101) {
        // D&D Dark Crimson / Dragon Theme
        Gdiplus::LinearGradientBrush linGrad(bannerRect,
            Gdiplus::Color(255, 55, 18, 14),
            Gdiplus::Color(255, 23, 26, 33),
            Gdiplus::LinearGradientModeForwardDiagonal
        );
        g.FillRectangle(&linGrad, bannerRect);

        Gdiplus::SolidBrush runeBrush(Gdiplus::Color(65, 240, 185, 45));
        Gdiplus::Font runeFont(L"Segoe UI", 28.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        g.DrawString(L"⚔  DUNGEONS & DRAGONS  ⚔", -1, &runeFont, Gdiplus::PointF(static_cast<float>(x + 30), static_cast<float>(y + 15)), &runeBrush);

        Gdiplus::SolidBrush subtitleBrush(Gdiplus::Color(180, 220, 180, 120));
        Gdiplus::Font subFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleItalic, Gdiplus::UnitPoint);
        g.DrawString(L"5e Edition Text-Based Role Playing Engine  •  ASCII Graphics  •  Tavern & Merchant Bob", -1, &subFont, Gdiplus::PointF(static_cast<float>(x + 35), static_cast<float>(y + 65)), &subtitleBrush);
    } else if (app.id == 570) {
        // Counter-Strike 2 / Tactical FPS: Warm Sand & Blue Sky gradient
        Gdiplus::LinearGradientBrush linGrad(bannerRect,
            Gdiplus::Color(255, 40, 55, 75),
            Gdiplus::Color(255, 23, 26, 33),
            Gdiplus::LinearGradientModeForwardDiagonal
        );
        g.FillRectangle(&linGrad, bannerRect);

        Gdiplus::SolidBrush csBrush(Gdiplus::Color(60, 235, 180, 85));
        Gdiplus::Font csFont(L"Segoe UI", 28.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        g.DrawString(L"︻╦デ═—  COUNTER-STRIKE 2", -1, &csFont, Gdiplus::PointF(static_cast<float>(x + 30), static_cast<float>(y + 15)), &csBrush);

        Gdiplus::SolidBrush subtitleBrush(Gdiplus::Color(190, 210, 225, 240));
        Gdiplus::Font subFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleItalic, Gdiplus::UnitPoint);
        g.DrawString(L"3D FPS Engine on C# WinForms  •  Raycasting DDA  •  Dust 2 Demake with Weapons & Bots", -1, &subFont, Gdiplus::PointF(static_cast<float>(x + 35), static_cast<float>(y + 65)), &subtitleBrush);
    } else if (app.id == 730) {
        // Half-Life 3 City 17 Combine Orange
        Gdiplus::LinearGradientBrush linGrad(bannerRect,
            Gdiplus::Color(255, 45, 30, 15),
            Gdiplus::Color(255, 23, 26, 33),
            Gdiplus::LinearGradientModeHorizontal
        );
        g.FillRectangle(&linGrad, bannerRect);

        Gdiplus::SolidBrush hlBrush(Gdiplus::Color(60, 255, 140, 20));
        Gdiplus::Font hlFont(L"Segoe UI", 28.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        g.DrawString(L"λ  HALF-LIFE 3  λ", -1, &hlFont, Gdiplus::PointF(static_cast<float>(x + 30), static_cast<float>(y + 15)), &hlBrush);
    } else {
        // Default Atmospheric Steam Wallpaper
        Gdiplus::LinearGradientBrush linGrad(bannerRect,
            Gdiplus::Color(255, 30, 50, 72),
            Gdiplus::Color(255, 23, 26, 33),
            Gdiplus::LinearGradientModeVertical
        );
        g.FillRectangle(&linGrad, bannerRect);
    }

    // Soft dark gradient overlay for bottom blending
    Gdiplus::Rect overlayRect(x, y + h - 60, w, 60);
    Gdiplus::LinearGradientBrush fadeGrad(overlayRect,
        Gdiplus::Color(0, 23, 26, 33),
        Gdiplus::Color(255, 23, 26, 33),
        Gdiplus::LinearGradientModeVertical
    );
    g.FillRectangle(&fadeGrad, overlayRect);
}

void SteamMainWindow::drawLibraryTab(Gdiplus::Graphics& g, const RECT& r) {
    constexpr int sidebarW = 270;
    const int contentW = r.right - sidebarW;

    // 1. Left Sidebar
    Gdiplus::SolidBrush sideBg(Gdiplus::Color(255, 28, 34, 42));
    g.FillRectangle(&sideBg, 0, 64, sidebarW, r.bottom - 64 - 32);

    // Interactive Search Box
    Gdiplus::SolidBrush searchBg(Gdiplus::Color(255, 40, 48, 58));
    g.FillRectangle(&searchBg, 12, 72, sidebarW - 24, 26);

    // Active / Hover border on search box
    if (m_isSearchActive || m_hoveredSearch) {
        Gdiplus::Pen activePen(Gdiplus::Color(255, 102, 192, 244), 1.5f);
        g.DrawRectangle(&activePen, 12, 72, sidebarW - 24, 26);
    }

    Gdiplus::Font searchFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush searchTxt(Gdiplus::Color(255, 220, 230, 240));
    Gdiplus::SolidBrush placeholderTxt(Gdiplus::Color(255, 130, 140, 150));

    if (m_searchQuery.empty()) {
        std::wstring ph = m_isSearchActive ? (m_cursorBlink ? L"| " : L"  ") : L"🔍  Поиск по играм...";
        g.DrawString(ph.c_str(), -1, &searchFont, Gdiplus::PointF(20, 76), &placeholderTxt);
    } else {
        std::wstring displayText = L"🔍 " + m_searchQuery;
        if (m_isSearchActive && m_cursorBlink) displayText += L"|";
        g.DrawString(displayText.c_str(), -1, &searchFont, Gdiplus::PointF(20, 76), &searchTxt);

        // Clear ✖ button
        g.DrawString(L"✖", -1, &searchFont, Gdiplus::PointF(static_cast<float>(sidebarW - 35), 76), &placeholderTxt);
    }

    // Sidebar Header: ИГРЫ И ПРОГРАММЫ
    Gdiplus::Font catFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush catTxt(Gdiplus::Color(255, 110, 125, 140));
    std::wstring countStr = L"Игры и Программы (" + std::to_wstring(m_filteredIndices.size()) + L")";
    g.DrawString(countStr.c_str(), -1, &catFont, Gdiplus::PointF(16, 108), &catTxt);

    // Games List
    int itemY = 130;
    Gdiplus::Font gameItemFont(L"Segoe UI", 9.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::Font gameBoldFont(L"Segoe UI", 9.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font itemSubFont(L"Segoe UI", 8.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);

    if (m_filteredIndices.empty()) {
        Gdiplus::SolidBrush noMatchTxt(Gdiplus::Color(255, 140, 145, 150));
        g.DrawString(L"Игры не найдены", -1, &gameItemFont, Gdiplus::PointF(24, static_cast<float>(itemY + 10)), &noMatchTxt);
    }

    for (std::size_t fi = 0; fi < m_filteredIndices.size(); ++fi) {
        std::size_t appIndex = m_filteredIndices[fi];
        const auto& app = m_apps[appIndex];
        const bool isSel = (appIndex == m_selectedIndex);
        const bool isHovered = (static_cast<int>(fi) == m_hoveredSidebarIndex);

        if (isSel) {
            Gdiplus::SolidBrush selBg(Gdiplus::Color(255, 52, 65, 80));
            g.FillRectangle(&selBg, 6, itemY, sidebarW - 12, 42);
        } else if (isHovered) {
            Gdiplus::SolidBrush hovBg(Gdiplus::Color(255, 38, 46, 56));
            g.FillRectangle(&hovBg, 6, itemY, sidebarW - 12, 42);
        }

        // Status indicator square
        Gdiplus::SolidBrush iconBrush(app.state == steam::core::AppState::Installed ?
            Gdiplus::Color(255, 102, 192, 244) : Gdiplus::Color(255, 100, 105, 110));
        g.FillRectangle(&iconBrush, 14, itemY + 8, 10, 10);

        // Title
        std::wstring titleW = utf8ToWide(app.title);
        Gdiplus::SolidBrush titleTxt(isSel ? Gdiplus::Color(255, 255, 255, 255) : Gdiplus::Color(255, 195, 205, 215));
        g.DrawString(titleW.c_str(), -1, isSel ? &gameBoldFont : &gameItemFont, Gdiplus::PointF(32, static_cast<float>(itemY + 4)), &titleTxt);

        // Subtitle status
        if (m_isGameRunning && m_runningAppId == app.id) {
            Gdiplus::SolidBrush runTxt(Gdiplus::Color(255, 164, 208, 7));
            g.DrawString(L"Запущено", -1, &itemSubFont, Gdiplus::PointF(32, static_cast<float>(itemY + 22)), &runTxt);
        } else if (app.state == steam::core::AppState::Installed) {
            Gdiplus::SolidBrush instTxt(Gdiplus::Color(255, 140, 160, 180));
            g.DrawString(L"Готово к игре", -1, &itemSubFont, Gdiplus::PointF(32, static_cast<float>(itemY + 22)), &instTxt);
        } else {
            Gdiplus::SolidBrush notInst(Gdiplus::Color(255, 110, 115, 120));
            g.DrawString(L"Не установлено", -1, &itemSubFont, Gdiplus::PointF(32, static_cast<float>(itemY + 22)), &notInst);
        }

        itemY += 46;
    }

    // 2. Right Content Area: Game Details with Hero Wallpaper
    if (m_selectedIndex < m_apps.size()) {
        const auto& curApp = m_apps[m_selectedIndex];

        // Draw Hero Wallpaper
        drawWallpaper(g, sidebarW, 64, contentW, 210, curApp);

        // Game Title
        Gdiplus::Font titleFont(L"Segoe UI", 22.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush titleBrush(Gdiplus::Color(255, 255, 255, 255));
        std::wstring curTitleW = utf8ToWide(curApp.title);
        g.DrawString(curTitleW.c_str(), -1, &titleFont, Gdiplus::PointF(static_cast<float>(sidebarW + 30), 125.0f), &titleBrush);

        // Action Button: [ ИГРАТЬ ] / [ УСТАНОВИТЬ ] / [ ОСТАНОВИТЬ ]
        Gdiplus::Rect btnRect(sidebarW + 30, 195, 200, 50);

        if (m_isGameRunning && m_runningAppId == curApp.id) {
            Gdiplus::SolidBrush stopBrush(m_hoveredPlayBtn ? Gdiplus::Color(255, 220, 55, 55) : Gdiplus::Color(255, 195, 45, 45));
            g.FillRectangle(&stopBrush, btnRect);

            Gdiplus::Font btnFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush btnTxt(Gdiplus::Color(255, 255, 255, 255));
            g.DrawString(L"■  ОСТАНОВИТЬ", -1, &btnFont, Gdiplus::PointF(static_cast<float>(btnRect.X + 28), static_cast<float>(btnRect.Y + 14)), &btnTxt);
        } else if (curApp.state == steam::core::AppState::Installed) {
            Gdiplus::Color g1 = m_hoveredPlayBtn ? Gdiplus::Color(255, 138, 215, 30) : Gdiplus::Color(255, 117, 185, 24);
            Gdiplus::Color g2 = m_hoveredPlayBtn ? Gdiplus::Color(255, 100, 165, 20) : Gdiplus::Color(255, 87, 145, 15);
            Gdiplus::LinearGradientBrush playBrush(btnRect, g1, g2, Gdiplus::LinearGradientModeVertical);
            g.FillRectangle(&playBrush, btnRect);

            Gdiplus::Font btnFont(L"Segoe UI", 13.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush btnTxt(Gdiplus::Color(255, 255, 255, 255));
            g.DrawString(L"▶  ИГРАТЬ", -1, &btnFont, Gdiplus::PointF(static_cast<float>(btnRect.X + 50), static_cast<float>(btnRect.Y + 14)), &btnTxt);
        } else if (m_isDownloading) {
            Gdiplus::SolidBrush downBrush(Gdiplus::Color(255, 38, 70, 105));
            g.FillRectangle(&downBrush, btnRect);

            Gdiplus::Font btnFont(L"Segoe UI", 11.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush btnTxt(Gdiplus::Color(255, 102, 192, 244));
            g.DrawString(L"СКАЧИВАНИЕ...", -1, &btnFont, Gdiplus::PointF(static_cast<float>(btnRect.X + 35), static_cast<float>(btnRect.Y + 14)), &btnTxt);
        } else {
            Gdiplus::LinearGradientBrush instBrush(btnRect,
                Gdiplus::Color(255, 50, 110, 180),
                Gdiplus::Color(255, 35, 80, 140),
                Gdiplus::LinearGradientModeVertical
            );
            g.FillRectangle(&instBrush, btnRect);

            Gdiplus::Font btnFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush btnTxt(Gdiplus::Color(255, 255, 255, 255));
            g.DrawString(L"УСТАНОВИТЬ", -1, &btnFont, Gdiplus::PointF(static_cast<float>(btnRect.X + 42), static_cast<float>(btnRect.Y + 14)), &btnTxt);
        }

        // Stats Row
        Gdiplus::Font statTitleFont(L"Segoe UI", 8.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
        Gdiplus::Font statValFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush statTitleTxt(Gdiplus::Color(255, 120, 135, 150));
        Gdiplus::SolidBrush statValTxt(Gdiplus::Color(255, 210, 220, 230));

        g.DrawString(L"ПОСЛЕДНИЙ ЗАПУСК", -1, &statTitleFont, Gdiplus::PointF(static_cast<float>(sidebarW + 260), 198), &statTitleTxt);
        g.DrawString(L"Сегодня", -1, &statValFont, Gdiplus::PointF(static_cast<float>(sidebarW + 260), 214), &statValTxt);

        uint64_t totalMins = (curApp.totalPlaytimeSeconds + (m_isGameRunning && m_runningAppId == curApp.id ? m_currentSessionSeconds : 0)) / 60;
        std::wstring playStr = std::to_wstring(totalMins) + L" мин.";
        if (m_isGameRunning && m_runningAppId == curApp.id) {
            playStr += L" (" + std::to_wstring(m_currentSessionSeconds) + L" сек. сейчас)";
        }
        g.DrawString(L"ВРЕМЯ В ИГРЕ", -1, &statTitleFont, Gdiplus::PointF(static_cast<float>(sidebarW + 410), 198), &statTitleTxt);
        g.DrawString(playStr.c_str(), -1, &statValFont, Gdiplus::PointF(static_cast<float>(sidebarW + 410), 214), &statValTxt);

        // Progress bar (if downloading)
        if (m_isDownloading) {
            Gdiplus::Rect track(sidebarW + 30, 260, 480, 14);
            Gdiplus::SolidBrush trackBg(Gdiplus::Color(255, 18, 22, 28));
            g.FillRectangle(&trackBg, track);

            int fillW = static_cast<int>(480 * m_downloadProgressFraction);
            Gdiplus::Rect fill(sidebarW + 30, 260, fillW, 14);
            Gdiplus::SolidBrush fillBg(Gdiplus::Color(255, 102, 192, 244));
            g.FillRectangle(&fillBg, fill);

            std::wostringstream progSs;
            progSs << L"Загрузка: " << std::fixed << std::setprecision(1)
                   << (m_downloadProgressFraction * 100.0f) << L"%  (" << m_downloadSpeedMbps << L" Mbps)";
            std::wstring ps = progSs.str();
            g.DrawString(ps.c_str(), -1, &statValFont, Gdiplus::PointF(static_cast<float>(sidebarW + 30), 280), &statValTxt);
        }

        // Section Cards below Wallpaper
        int cardsY = m_isDownloading ? 315 : 275;

        // Card 1: Description
        Gdiplus::Rect descRect(sidebarW + 30, cardsY, 480, 180);
        Gdiplus::SolidBrush cardBg(Gdiplus::Color(255, 26, 33, 44));
        g.FillRectangle(&cardBg, descRect);

        Gdiplus::Font cardHeadFont(L"Segoe UI", 11.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush cardHeadTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(L"ОБ ИГРЕ", -1, &cardHeadFont, Gdiplus::PointF(static_cast<float>(descRect.X + 16), static_cast<float>(descRect.Y + 12)), &cardHeadTxt);

        Gdiplus::Font bodyFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush bodyTxt(Gdiplus::Color(255, 180, 195, 210));

        if (curApp.id == 101) {
            std::wstring desc =
                L"Классическая текстовая ролевая игра по мотивам D&D 5-й редакции.\n"
                L"• Выбор рас (Человек, Эльф, Дварф, Полуорк, Драконорождённый...)\n"
                L"• 12 классов: Воин, Маг, Плут, Жрец, Варвар, Паладин и др.\n"
                L"• Инвентарь, экипировка, заклинания и зелья.\n"
                L"• Торговец Боб и интерактивная ASCII-арт энциклопедия.";
            g.DrawString(desc.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(descRect.X + 16), static_cast<float>(descRect.Y + 38)), &bodyTxt);
        } else if (curApp.id == 570) {
            std::wstring desc =
                L"Тактический 3D шутер от первого лица на C# Windows Forms.\n"
                L"• Кастомный софтверный DDA Raycaster с прямой записью в память.\n"
                L"• Полный обзор во все стороны (Yaw + Pitch наклон камеры).\n"
                L"• Физика прыжков (Space) и присядки (Ctrl / C).\n"
                L"• Интерактивные двери [E] и карта Dust 2.\n"
                L"• Арсенал: AK-47, Desert Eagle, Боевой нож с хедшотами.";
            g.DrawString(desc.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(descRect.X + 16), static_cast<float>(descRect.Y + 38)), &bodyTxt);
        } else {
            std::wstring desc = L"Полный цифровой дистрибутив с поддержкой синхронизации и локального запуска.";
            g.DrawString(desc.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(descRect.X + 16), static_cast<float>(descRect.Y + 38)), &bodyTxt);
        }

        // Card 2: Achievements
        Gdiplus::Rect achRect(sidebarW + 530, cardsY, 300, 180);
        g.FillRectangle(&cardBg, achRect);
        g.DrawString(L"ДОСТИЖЕНИЯ", -1, &cardHeadFont, Gdiplus::PointF(static_cast<float>(achRect.X + 16), static_cast<float>(achRect.Y + 12)), &cardHeadTxt);

        std::wstring achDesc =
            L"Разблокировано: 4 из 12 (33%)\n\n"
            L"🏆 Первый запуск игры\n"
            L"🏆 Точный выстрел в голову\n"
            L"🏆 Исследователь локаций\n"
            L"🏆 Ветеран Steam Clone";
        g.DrawString(achDesc.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(achRect.X + 16), static_cast<float>(achRect.Y + 38)), &bodyTxt);
    }
}

void SteamMainWindow::drawStoreTab(Gdiplus::Graphics& g, const RECT& r) {
    // Top Subnav
    Gdiplus::SolidBrush storeNavBg(Gdiplus::Color(255, 30, 38, 48));
    g.FillRectangle(&storeNavBg, 20, 68, r.right - 40, 36);

    Gdiplus::Font navFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush navTxt(Gdiplus::Color(255, 215, 225, 235));
    g.DrawString(L"Просмотр ▼    Рекомендации ▼    Категории ▼    Прочее ▼", -1, &navFont, Gdiplus::PointF(35, 78), &navTxt);

    // Wishlist Button
    Gdiplus::SolidBrush wishBg(Gdiplus::Color(255, 26, 159, 255));
    g.DrawString(L"★ Список желаемого (18)", -1, &navFont, Gdiplus::PointF(static_cast<float>(r.right - 210), 78), &wishBg);

    // FEATURED CAROUSEL CARDS
    constexpr int cardW = 340;
    constexpr int cardH = 240;
    const int totalCardsW = cardW * 3 + 40;
    const int startX = std::max<int>(20, static_cast<int>(r.right - totalCardsW) / 2);

    // Left Arrow
    Gdiplus::SolidBrush arrowBrush(Gdiplus::Color(255, 140, 150, 160));
    Gdiplus::Font arrowBig(L"Segoe UI", 20.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    g.DrawString(L"◀", -1, &arrowBig, Gdiplus::PointF(static_cast<float>(startX - 40), 215), &arrowBrush);

    // Right Arrow
    g.DrawString(L"▶", -1, &arrowBig, Gdiplus::PointF(static_cast<float>(startX + totalCardsW + 10), 215), &arrowBrush);

    Gdiplus::Font gameHead(L"Segoe UI", 11.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font genreFont(L"Segoe UI", 8.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::Font discFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font priceOldFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleStrikeout, Gdiplus::UnitPoint);
    Gdiplus::Font priceNewFont(L"Segoe UI", 11.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);

    int currentX = startX;

    for (std::size_t i = 0; i < m_storeGames.size(); ++i) {
        const auto& fg = m_storeGames[i];

        // Card Container with hover border
        Gdiplus::Rect cRect(currentX, 120, cardW, cardH);
        Gdiplus::LinearGradientBrush cardBrush(cRect, fg.themeColor, Gdiplus::Color(255, 20, 26, 34), Gdiplus::LinearGradientModeVertical);
        g.FillRectangle(&cardBrush, cRect);

        // Check if game is in library
        bool inLibrary = false;
        for (const auto& a : m_apps) {
            if (a.id == fg.id) { inLibrary = true; break; }
        }

        // Title & Genre
        Gdiplus::SolidBrush whiteTxt(Gdiplus::Color(255, 255, 255, 255));
        Gdiplus::SolidBrush subTxt(Gdiplus::Color(255, 185, 195, 205));
        g.DrawString(fg.title.c_str(), -1, &gameHead, Gdiplus::PointF(static_cast<float>(currentX + 14), 135), &whiteTxt);
        g.DrawString(fg.genre.c_str(), -1, &genreFont, Gdiplus::PointF(static_cast<float>(currentX + 14), 160), &subTxt);

        // Description preview
        Gdiplus::Font descPreviewFont(L"Segoe UI", 8.0f, Gdiplus::FontStyleItalic, Gdiplus::UnitPoint);
        g.DrawString(fg.description.c_str(), -1, &descPreviewFont, Gdiplus::PointF(static_cast<float>(currentX + 14), 185), &subTxt);

        // Discount Tag (-75% Green Badge)
        Gdiplus::Rect discRect(currentX + 14, 310, 55, 26);
        Gdiplus::SolidBrush discBadge(Gdiplus::Color(255, 164, 208, 7));
        g.FillRectangle(&discBadge, discRect);

        Gdiplus::SolidBrush discTxt(Gdiplus::Color(255, 0, 0, 0));
        g.DrawString(fg.discount.c_str(), -1, &discFont, Gdiplus::PointF(static_cast<float>(discRect.X + 6), static_cast<float>(discRect.Y + 3)), &discTxt);

        // Prices
        Gdiplus::SolidBrush oldPriceTxt(Gdiplus::Color(255, 130, 140, 150));
        Gdiplus::SolidBrush newPriceTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(fg.oldPrice.c_str(), -1, &priceOldFont, Gdiplus::PointF(static_cast<float>(currentX + 78), 307), &oldPriceTxt);
        g.DrawString(fg.newPrice.c_str(), -1, &priceNewFont, Gdiplus::PointF(static_cast<float>(currentX + 78), 321), &newPriceTxt);

        // Action Button: [ В КОРЗИНУ ] / [ В БИБЛИОТЕКЕ ]
        Gdiplus::Rect buyBtnRect(currentX + cardW - 135, 308, 120, 30);
        if (inLibrary) {
            Gdiplus::SolidBrush inLibBrush(Gdiplus::Color(255, 45, 60, 75));
            g.FillRectangle(&inLibBrush, buyBtnRect);
            Gdiplus::Font smallBtnFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush inLibTxt(Gdiplus::Color(255, 102, 192, 244));
            g.DrawString(L"В БИБЛИОТЕКЕ", -1, &smallBtnFont, Gdiplus::PointF(static_cast<float>(buyBtnRect.X + 10), static_cast<float>(buyBtnRect.Y + 7)), &inLibTxt);
        } else {
            Gdiplus::LinearGradientBrush buyBrush(buyBtnRect,
                Gdiplus::Color(255, 117, 185, 24),
                Gdiplus::Color(255, 87, 145, 15),
                Gdiplus::LinearGradientModeVertical);
            g.FillRectangle(&buyBrush, buyBtnRect);
            Gdiplus::Font smallBtnFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush buyTxt(Gdiplus::Color(255, 255, 255, 255));
            g.DrawString(L"+ В КОРЗИНУ", -1, &smallBtnFont, Gdiplus::PointF(static_cast<float>(buyBtnRect.X + 16), static_cast<float>(buyBtnRect.Y + 7)), &buyTxt);
        }

        currentX += cardW + 20;
    }

    // SECTION 2: Крупные скидки на избранные игры
    int sec2Y = 385;
    Gdiplus::Font secTitle(L"Segoe UI", 13.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush secTxt(Gdiplus::Color(255, 255, 255, 255));
    g.DrawString(L"Крупные скидки на избранные игры (Нажмите для добавления)", -1, &secTitle, Gdiplus::PointF(static_cast<float>(startX), static_cast<float>(sec2Y)), &secTxt);

    currentX = startX;
    for (const auto& dg : m_storeDiscounts) {
        Gdiplus::Rect cRect(currentX, sec2Y + 30, cardW, 140);
        Gdiplus::LinearGradientBrush cardBrush(cRect, dg.themeColor, Gdiplus::Color(255, 20, 24, 30), Gdiplus::LinearGradientModeVertical);
        g.FillRectangle(&cardBrush, cRect);

        bool inLibrary = false;
        for (const auto& a : m_apps) {
            if (a.id == dg.id) { inLibrary = true; break; }
        }

        Gdiplus::SolidBrush whiteTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(dg.title.c_str(), -1, &gameHead, Gdiplus::PointF(static_cast<float>(currentX + 12), static_cast<float>(sec2Y + 40)), &whiteTxt);

        // Discount Tag
        Gdiplus::Rect discRect(currentX + 12, sec2Y + 125, 52, 24);
        Gdiplus::SolidBrush discBadge(Gdiplus::Color(255, 164, 208, 7));
        g.FillRectangle(&discBadge, discRect);
        Gdiplus::SolidBrush discTxt(Gdiplus::Color(255, 0, 0, 0));
        g.DrawString(dg.discount.c_str(), -1, &discFont, Gdiplus::PointF(static_cast<float>(discRect.X + 4), static_cast<float>(discRect.Y + 2)), &discTxt);

        // Price tags
        Gdiplus::SolidBrush oldPriceTxt(Gdiplus::Color(255, 120, 130, 140));
        Gdiplus::SolidBrush newPriceTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(dg.oldPrice.c_str(), -1, &priceOldFont, Gdiplus::PointF(static_cast<float>(currentX + 72), static_cast<float>(sec2Y + 123)), &oldPriceTxt);
        g.DrawString(dg.newPrice.c_str(), -1, &priceNewFont, Gdiplus::PointF(static_cast<float>(currentX + 72), static_cast<float>(sec2Y + 135)), &newPriceTxt);

        // Button
        Gdiplus::Rect buyBtn(currentX + cardW - 120, sec2Y + 125, 108, 26);
        if (inLibrary) {
            Gdiplus::SolidBrush inLibBrush(Gdiplus::Color(255, 45, 60, 75));
            g.FillRectangle(&inLibBrush, buyBtn);
            Gdiplus::Font smallBtn(L"Segoe UI", 8.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush inLibTxt(Gdiplus::Color(255, 102, 192, 244));
            g.DrawString(L"В БИБЛИОТЕКЕ", -1, &smallBtn, Gdiplus::PointF(static_cast<float>(buyBtn.X + 8), static_cast<float>(buyBtn.Y + 5)), &inLibTxt);
        } else {
            Gdiplus::LinearGradientBrush buyBrush(buyBtn,
                Gdiplus::Color(255, 117, 185, 24),
                Gdiplus::Color(255, 87, 145, 15),
                Gdiplus::LinearGradientModeVertical);
            g.FillRectangle(&buyBrush, buyBtn);
            Gdiplus::Font smallBtn(L"Segoe UI", 8.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush buyTxt(Gdiplus::Color(255, 255, 255, 255));
            g.DrawString(L"+ КУПИТЬ", -1, &smallBtn, Gdiplus::PointF(static_cast<float>(buyBtn.X + 22), static_cast<float>(buyBtn.Y + 5)), &buyTxt);
        }

        currentX += cardW + 20;
    }
}

void SteamMainWindow::drawCommunityTab(Gdiplus::Graphics& g, const RECT& r) {
    Gdiplus::Font headFont(L"Segoe UI", 16.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font bodyFont(L"Segoe UI", 9.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush whiteTxt(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush subTxt(Gdiplus::Color(255, 170, 185, 200));

    g.DrawString(L"Сообщество Steam — Центр активности и модификаций", -1, &headFont, Gdiplus::PointF(40, 80), &whiteTxt);

    // Card 1
    Gdiplus::Rect card1(40, 125, r.right - 80, 110);
    Gdiplus::SolidBrush cardBg(Gdiplus::Color(255, 28, 36, 48));
    g.FillRectangle(&cardBg, card1);

    Gdiplus::Font cardTitle(L"Segoe UI", 11.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    g.DrawString(L"💬 Dungeons & Dragons: Console RPG — Обсуждения сообщества", -1, &cardTitle, Gdiplus::PointF(static_cast<float>(card1.X + 16), static_cast<float>(card1.Y + 14)), &whiteTxt);

    std::wstring text1 =
        L"Автор: GoblinSlayer99  •  Сообщений: 142\n"
        L"«Как лучше всего распределять очки характеристик для Паладина-полуэльфа, чтобы выдержать атаку дракона? "
        L"Делитесь своими билдами и тактиками!»";
    g.DrawString(text1.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(card1.X + 16), static_cast<float>(card1.Y + 42)), &subTxt);

    // Card 2
    Gdiplus::Rect card2(40, 255, r.right - 80, 110);
    g.FillRectangle(&cardBg, card2);

    g.DrawString(L"🛠 Мастерская Steam — Популярные модификации", -1, &cardTitle, Gdiplus::PointF(static_cast<float>(card2.X + 16), static_cast<float>(card2.Y + 14)), &whiteTxt);

    std::wstring text2 =
        L"Мод: «Расширенный бестиарий и 15 легендарных артефактов» (v1.4)\n"
        L"Рейтинг: ★★★★★ (1,290 подписчиков)\n"
        L"Добавляет уникальных монстров в пещеры и новый инвентарь для торговца Боба.";
    g.DrawString(text2.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(card2.X + 16), static_cast<float>(card2.Y + 42)), &subTxt);

    // Card 3
    Gdiplus::Rect card3(40, 385, r.right - 80, 110);
    g.FillRectangle(&cardBg, card3);

    g.DrawString(L"📷 Галерея скриншотов и ASCII-Арта", -1, &cardTitle, Gdiplus::PointF(static_cast<float>(card3.X + 16), static_cast<float>(card3.Y + 14)), &whiteTxt);

    std::wstring text3 =
        L"Пользователь: DragonKnight  •  «ASCII-Арт финального босса в полном разрешении!»\n"
        L"Понравилось 418 игрокам  •  12 наград сообщества.";
    g.DrawString(text3.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(card3.X + 16), static_cast<float>(card3.Y + 42)), &subTxt);
}

void SteamMainWindow::drawBottomBar(Gdiplus::Graphics& g, const RECT& r) {
    Gdiplus::SolidBrush botBg(Gdiplus::Color(255, 20, 23, 29));
    g.FillRectangle(&botBg, 0, r.bottom - 32, r.right, 32);

    Gdiplus::Font botFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush botTxt(Gdiplus::Color(255, 145, 155, 165));
    Gdiplus::SolidBrush activeBotTxt(Gdiplus::Color(255, 102, 192, 244));

    // + Добавить игру (with hover highlight)
    g.DrawString(L"➕ Добавить игру", -1, &botFont, Gdiplus::PointF(16, static_cast<float>(r.bottom - 24)), m_hoveredAddGame ? &activeBotTxt : &botTxt);

    // Управление загрузками
    std::wstring downText = L"⬇ Управление загрузками";
    if (m_isDownloading) {
        downText += L" (1 активно)";
    }
    g.DrawString(downText.c_str(), -1, &botFont, Gdiplus::PointF(static_cast<float>(r.right / 2 - 80), static_cast<float>(r.bottom - 24)), &botTxt);

    // Друзья и чат
    g.DrawString(L"👥 Друзья и чат (3)", -1, &botFont, Gdiplus::PointF(static_cast<float>(r.right - 145), static_cast<float>(r.bottom - 24)), &botTxt);
}

void SteamMainWindow::drawToast(Gdiplus::Graphics& g, const RECT& r) {
    if (m_toastTimer <= 0 || m_toastMessage.empty()) return;

    Gdiplus::Font toastFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush toastBg(Gdiplus::Color(240, 24, 38, 54));
    Gdiplus::Pen toastBorder(Gdiplus::Color(255, 102, 192, 244), 1.5f);
    Gdiplus::SolidBrush toastTxt(Gdiplus::Color(255, 255, 255, 255));

    int toastW = 340;
    int toastH = 46;
    int toastX = r.right - toastW - 20;
    int toastY = r.bottom - toastH - 45;

    Gdiplus::Rect tRect(toastX, toastY, toastW, toastH);
    g.FillRectangle(&toastBg, tRect);
    g.DrawRectangle(&toastBorder, tRect);

    g.DrawString(m_toastMessage.c_str(), -1, &toastFont, Gdiplus::PointF(static_cast<float>(toastX + 15), static_cast<float>(toastY + 12)), &toastTxt);
}

int SteamMainWindow::runEventLoop() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

} // namespace steam::ui
#endif
