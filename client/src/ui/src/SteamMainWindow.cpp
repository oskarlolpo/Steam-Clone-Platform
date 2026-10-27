#include "steam/ui/SteamMainWindow.hpp"
#include <algorithm>
#include <chrono>
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

SteamMainWindow::SteamMainWindow(
    std::shared_ptr<steam::storage::ILocalStorage> storage,
    std::shared_ptr<steam::downloader::IDownloadManager> downloadManager,
    std::shared_ptr<steam::launcher::IProcessLauncher> processLauncher
) : m_storage(std::move(storage)),
    m_downloadManager(std::move(downloadManager)),
    m_processLauncher(std::move(processLauncher))
{
    // Initialize GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    ensureGamesInitialized();
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

    // Ensure D&D game from C:\Users\govno\Desktop\zadaniy\ind is in the library
    bool hasDnd = false;
    for (auto& app : m_apps) {
        if (app.id == 101 || app.title.find("Dungeons & Dragons") != std::string::npos || app.title.find("D&D") != std::string::npos) {
            hasDnd = true;
            // Ensure proper path
            app.id = 101;
            app.title = "Dungeons & Dragons: Console RPG";
            app.version = "1.0.0 (Release)";
            app.installDirectory = "C:\\Users\\govno\\Desktop\\zadaniy\\ind\\Console-DnD-main";
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
        dndGame.installDirectory = "C:\\Users\\govno\\Desktop\\zadaniy\\ind\\Console-DnD-main";
        dndGame.executableRelativePath = "game.exe";
        dndGame.downloadUrl = "https://cdn.steam-clone.local/depot/dnd_rpg.bin";
        dndGame.state = steam::core::AppState::Installed;
        dndGame.totalPlaytimeSeconds = 2520; // 42 mins

        m_storage->saveAppManifest(dndGame);
    }

    // Add classic games if list is small
    if (m_apps.size() < 3) {
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

        steam::core::AppManifest cs2;
        cs2.id = 570;
        cs2.title = "Counter-Strike 2: Source Edition";
        cs2.version = "2.1.0";
        cs2.installSize = 25 * 1024 * 1024;
        cs2.installDirectory = "steamapps/common/CS2Source";
        cs2.executableRelativePath = "cs2.exe";
        cs2.downloadUrl = "https://cdn.steam-clone.local/depot/cs2.bin";
        cs2.state = steam::core::AppState::Installed;
        cs2.totalPlaytimeSeconds = 14200;
        m_storage->saveAppManifest(cs2);

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

    // Default select D&D game
    for (std::size_t i = 0; i < m_apps.size(); ++i) {
        if (m_apps[i].id == 101) {
            m_selectedIndex = i;
            break;
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
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, width, height,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!m_hwnd) {
        return false;
    }

    // Set clean icon and title
    SetWindowTextW(m_hwnd, L"Steam");

    // GDI Fonts
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

    // Timer: updates game playtime and animations every second
    SetTimer(m_hwnd, 1, 1000, nullptr);

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

        case WM_SETCURSOR: {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);

            bool isClickable = false;
            // Tabs
            if (pt.y >= 24 && pt.y <= 58 && pt.x >= 65 && pt.x <= 430) isClickable = true;
            // Action button
            if (pt.x >= 300 && pt.x <= 500 && pt.y >= 195 && pt.y <= 245) isClickable = true;
            // Sidebar items
            if (m_currentTab == MainTab::Library && pt.x < 270 && pt.y >= 130 && pt.y < 130 + static_cast<int>(m_apps.size() * 46)) isClickable = true;
            // Store carousel arrows
            if (m_currentTab == MainTab::Store && pt.y >= 200 && pt.y <= 270 && (pt.x <= 55 || pt.x >= 1045)) isClickable = true;

            if (isClickable) {
                SetCursor(LoadCursor(nullptr, IDC_HAND));
                return TRUE;
            }
            break;
        }

        case WM_LBUTTONDOWN: {
            const int x = LOWORD(lParam);
            const int y = HIWORD(lParam);
            onClick(x, y);
            return 0;
        }

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
            } else {
                m_isGameRunning = false;
                m_statusMessage = "Игра закрыта";
                m_apps = m_storage->getAllApps();
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
    if (m_isGameRunning) {
        m_currentSessionSeconds++;
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void SteamMainWindow::onClick(int x, int y) {
    // 1. Top Main Tab Navigation
    // Coordinates: Y between 24 and 58
    if (y >= 24 && y <= 58) {
        if (x >= 65 && x <= 165) {
            m_currentTab = MainTab::Store;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        } else if (x >= 170 && x <= 295) {
            m_currentTab = MainTab::Library;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        } else if (x >= 300 && x <= 430) {
            m_currentTab = MainTab::Community;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }
    }

    // 2. Tab Specific Clicks
    if (m_currentTab == MainTab::Library) {
        // Sidebar item click (x < 270, y >= 130)
        constexpr int sidebarWidth = 270;
        constexpr int listStartY = 130;
        constexpr int itemH = 46;

        if (x < sidebarWidth && y >= listStartY) {
            const std::size_t idx = static_cast<std::size_t>((y - listStartY) / itemH);
            if (idx < m_apps.size()) {
                m_selectedIndex = idx;
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return;
            }
        }

        // Action Button: [ ИГРАТЬ ] / [ УСТАНОВИТЬ ] / [ ОСТАНОВИТЬ ]
        // Button Position: x: 300, y: 195, w: 200, h: 50
        const RECT btnRect{ 300, 195, 500, 245 };
        if (x >= btnRect.left && x <= btnRect.right && y >= btnRect.top && y <= btnRect.bottom) {
            if (m_selectedIndex >= m_apps.size()) return;
            auto& app = m_apps[m_selectedIndex];

            if (m_isGameRunning) {
                m_processLauncher->terminate(app.id, true);
            } else if (app.state == steam::core::AppState::Installed) {
                // Launch game
                steam::launcher::LaunchConfig config;
                config.createNewConsole = true;

                if (app.id == 101) {
                    // D&D Console RPG
                    config.executablePath = "C:\\Users\\govno\\Desktop\\zadaniy\\ind\\Console-DnD-main\\game.exe";
                    config.workingDirectory = "C:\\Users\\govno\\Desktop\\zadaniy\\ind\\Console-DnD-main";
                } else if (app.id == 570) {
                    // Counter-Strike 2 / Tactical FPS Demo
                    std::filesystem::path cs2Local = std::filesystem::current_path() / "steamapps" / "common" / "CS2Source" / "cs2.exe";
                    if (std::filesystem::exists(cs2Local)) {
                        config.executablePath = cs2Local;
                        config.workingDirectory = cs2Local.parent_path();
                    } else {
                        config.executablePath = "C:\\Users\\govno\\Desktop\\zadaniy\\CS2WinFormsDemo\\bin\\Release\\publish\\CS2WinFormsDemo.exe";
                        config.workingDirectory = "C:\\Users\\govno\\Desktop\\zadaniy\\CS2WinFormsDemo\\bin\\Release\\publish";
                    }
                    config.createNewConsole = false;
                } else {
                    std::filesystem::path fullExe = app.installDirectory.is_absolute()
                        ? (app.installDirectory / app.executableRelativePath)
                        : (std::filesystem::current_path() / app.installDirectory / app.executableRelativePath);

                    if (std::filesystem::exists(fullExe)) {
                        config.executablePath = fullExe;
                        config.workingDirectory = fullExe.parent_path();
                        config.createNewConsole = false;
                    } else {
                        // Fallback simulated game via cmd
                        config.executablePath = "C:\\Windows\\System32\\cmd.exe";
                        config.workingDirectory = std::filesystem::current_path();
                        config.arguments = {"/c", "echo Steam Game Launched! & ping -n 60 127.0.0.1 >nul"};
                    }
                }

                if (m_processLauncher->launch(app.id, config)) {
                    const auto info = m_processLauncher->getInfo(app.id);
                    m_runningPid = info.pid;
                    m_runningAppId = app.id;
                    m_isGameRunning = true;
                    m_currentSessionSeconds = 0;
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
            }
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }
    } else if (m_currentTab == MainTab::Store) {
        // Carousel arrows click:
        // Left arrow: x: 20, y: 220, w: 35, h: 60
        if (x >= 20 && x <= 55 && y >= 200 && y <= 270) {
            m_storeCarouselIndex = (m_storeCarouselIndex + 2) % 3;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
        }
        // Right arrow: x: 1045, y: 220, w: 35, h: 60
        if (x >= 1045 && x <= 1080 && y >= 200 && y <= 270) {
            m_storeCarouselIndex = (m_storeCarouselIndex + 1) % 3;
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return;
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

    // 1. Base dark background (#171a21)
    Gdiplus::SolidBrush bgBrush(Gdiplus::Color(255, 23, 26, 33));
    g.FillRectangle(&bgBrush, 0, 0, r.right, r.bottom);

    // 2. Top Navigation Bar
    drawTopNav(g, r);

    // 3. Tab Content
    if (m_currentTab == MainTab::Store) {
        drawStoreTab(g, r);
    } else if (m_currentTab == MainTab::Library) {
        drawLibraryTab(g, r);
    } else {
        drawCommunityTab(g, r);
    }

    // 4. Bottom status footer
    drawBottomBar(g, r);

    // Blit to screen
    BitBlt(hdc, 0, 0, r.right, r.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(hwnd, &ps);
}

void SteamMainWindow::drawTopNav(Gdiplus::Graphics& g, const RECT& r) {
    // Menu bar at the very top (Steam, Вид, Друзья, Игры, Справка)
    Gdiplus::SolidBrush menuBg(Gdiplus::Color(255, 23, 26, 33));
    g.FillRectangle(&menuBg, 0, 0, r.right, 24);

    Gdiplus::Font menuFont(L"Segoe UI", 8.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush menuTextBrush(Gdiplus::Color(255, 170, 180, 190));
    g.DrawString(L"Steam   Вид   Друзья   Игры   Справка", -1, &menuFont, Gdiplus::PointF(16, 4), &menuTextBrush);

    // Subnav Bar (#171a21 to #1b2838)
    Gdiplus::SolidBrush subnavBg(Gdiplus::Color(255, 23, 26, 33));
    g.FillRectangle(&subnavBg, 0, 24, r.right, 40);

    // Navigation Arrows: ⬅ ➡
    Gdiplus::Font arrowFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush arrowBrush(Gdiplus::Color(255, 130, 140, 150));
    g.DrawString(L"◀   ▶", -1, &arrowFont, Gdiplus::PointF(16, 30), &arrowBrush);

    // Big Tabs: МАГАЗИН, БИБЛИОТЕКА, СООБЩЕСТВО
    Gdiplus::Font tabFont(L"Segoe UI", 11.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);

    // Colors based on active tab
    auto getTabBrush = [&](MainTab tab) -> Gdiplus::SolidBrush {
        if (m_currentTab == tab) {
            return Gdiplus::SolidBrush(Gdiplus::Color(255, 26, 159, 255)); // Active bright cyan / blue
        }
        return Gdiplus::SolidBrush(Gdiplus::Color(255, 210, 220, 230)); // Inactive light text
    };

    auto bStore = getTabBrush(MainTab::Store);
    auto bLib = getTabBrush(MainTab::Library);
    auto bComm = getTabBrush(MainTab::Community);

    g.DrawString(L"МАГАЗИН", -1, &tabFont, Gdiplus::PointF(70, 31), &bStore);
    g.DrawString(L"БИБЛИОТЕКА", -1, &tabFont, Gdiplus::PointF(180, 31), &bLib);
    g.DrawString(L"СООБЩЕСТВО", -1, &tabFont, Gdiplus::PointF(315, 31), &bComm);

    // Draw active tab indicator underline
    Gdiplus::Pen activePen(Gdiplus::Color(255, 26, 159, 255), 3.0f);
    if (m_currentTab == MainTab::Store) {
        g.DrawLine(&activePen, 70, 56, 145, 56);
    } else if (m_currentTab == MainTab::Library) {
        g.DrawLine(&activePen, 180, 56, 280, 56);
    } else if (m_currentTab == MainTab::Community) {
        g.DrawLine(&activePen, 315, 56, 425, 56);
    }

    // Right side: User Profile info matching screenshot
    Gdiplus::SolidBrush userBrush(Gdiplus::Color(255, 164, 208, 7)); // Green badge
    g.DrawString(L"ØSKARŁØŁPØ⁴²   75,31 руб", -1, &tabFont, Gdiplus::PointF(static_cast<float>(r.right - 270), 31), &userBrush);
}

void SteamMainWindow::drawWallpaper(Gdiplus::Graphics& g, int x, int y, int w, int h, const steam::core::AppManifest& app) {
    // Modern Steam Hero Wallpaper (ОБОИ) with rich multi-stop gradient
    Gdiplus::Rect bannerRect(x, y, w, h);

    if (app.id == 101) {
        // D&D Console RPG: Epic Dark Fantasy Red/Gold/Dungeon Wallpaper
        Gdiplus::LinearGradientBrush linGrad(bannerRect,
            Gdiplus::Color(255, 50, 18, 12),   // Dark crimson dragon fire
            Gdiplus::Color(255, 20, 24, 32),   // Blends into Steam dark
            Gdiplus::LinearGradientModeForwardDiagonal
        );
        g.FillRectangle(&linGrad, bannerRect);

        // Ambient dungeon runes and accents
        Gdiplus::SolidBrush runeBrush(Gdiplus::Color(60, 240, 180, 40));
        Gdiplus::Font runeFont(L"Segoe UI", 28.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        g.DrawString(L"⚔  DUNGEONS & DRAGONS  ⚔", -1, &runeFont, Gdiplus::PointF(static_cast<float>(x + 30), static_cast<float>(y + 15)), &runeBrush);

        Gdiplus::SolidBrush subtitleBrush(Gdiplus::Color(180, 220, 180, 120));
        Gdiplus::Font subFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleItalic, Gdiplus::UnitPoint);
        g.DrawString(L"5e Edition Text-Based Role Playing Engine  •  ASCII Graphics  •  Tavern & Merchant Bob", -1, &subFont, Gdiplus::PointF(static_cast<float>(x + 35), static_cast<float>(y + 65)), &subtitleBrush);
    } else if (app.id == 730) {
        // Half-Life 3: City 17 Combine Orange/Slate Wallpaper
        Gdiplus::LinearGradientBrush linGrad(bannerRect,
            Gdiplus::Color(255, 40, 30, 20),
            Gdiplus::Color(255, 18, 22, 28),
            Gdiplus::LinearGradientModeHorizontal
        );
        g.FillRectangle(&linGrad, bannerRect);

        Gdiplus::SolidBrush hlBrush(Gdiplus::Color(60, 255, 140, 20));
        Gdiplus::Font hlFont(L"Segoe UI", 28.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        g.DrawString(L"λ  HALF-LIFE 3  λ", -1, &hlFont, Gdiplus::PointF(static_cast<float>(x + 30), static_cast<float>(y + 15)), &hlBrush);
    } else {
        // Default Atmospheric Steam Wallpaper
        Gdiplus::LinearGradientBrush linGrad(bannerRect,
            Gdiplus::Color(255, 30, 48, 68),
            Gdiplus::Color(255, 20, 24, 30),
            Gdiplus::LinearGradientModeVertical
        );
        g.FillRectangle(&linGrad, bannerRect);
    }

    // Soft dark gradient overlay to blend smoothly into content bottom
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
    Gdiplus::SolidBrush sideBg(Gdiplus::Color(255, 30, 36, 44));
    g.FillRectangle(&sideBg, 0, 64, sidebarW, r.bottom - 64 - 32);

    // Sidebar search box
    Gdiplus::SolidBrush searchBg(Gdiplus::Color(255, 42, 50, 60));
    g.FillRectangle(&searchBg, 12, 72, sidebarW - 24, 26);

    Gdiplus::Font searchFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush searchTxt(Gdiplus::Color(255, 140, 150, 160));
    g.DrawString(L"🔍  Поиск по играм...", -1, &searchFont, Gdiplus::PointF(20, 76), &searchTxt);

    // Sidebar Header: ИГРЫ И ПРОГРАММЫ
    Gdiplus::Font catFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush catTxt(Gdiplus::Color(255, 110, 125, 140));
    std::wstring countStr = L"Игры и Программы (" + std::to_wstring(m_apps.size()) + L")";
    g.DrawString(countStr.c_str(), -1, &catFont, Gdiplus::PointF(16, 108), &catTxt);

    // Games List
    int itemY = 130;
    Gdiplus::Font gameItemFont(L"Segoe UI", 9.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::Font gameBoldFont(L"Segoe UI", 9.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font itemSubFont(L"Segoe UI", 8.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);

    for (std::size_t i = 0; i < m_apps.size(); ++i) {
        const auto& app = m_apps[i];
        const bool isSel = (i == m_selectedIndex);

        if (isSel) {
            Gdiplus::SolidBrush selBg(Gdiplus::Color(255, 52, 65, 80));
            g.FillRectangle(&selBg, 6, itemY, sidebarW - 12, 42);
        }

        // Game icon marker
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

    // 2. Right Content Area: Game Details with Wallpaper (ОБОИ)
    if (m_selectedIndex < m_apps.size()) {
        const auto& curApp = m_apps[m_selectedIndex];

        // Draw Game Hero Wallpaper (Обои)
        drawWallpaper(g, sidebarW, 64, contentW, 210, curApp);

        // Game Title
        Gdiplus::Font titleFont(L"Segoe UI", 20.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush titleBrush(Gdiplus::Color(255, 255, 255, 255));
        std::wstring curTitleW = utf8ToWide(curApp.title);
        g.DrawString(curTitleW.c_str(), -1, &titleFont, Gdiplus::PointF(static_cast<float>(sidebarW + 30), 125.0f), &titleBrush);

        // Action Button: [ ИГРАТЬ ] / [ УСТАНОВИТЬ ] / [ ОСТАНОВИТЬ ]
        Gdiplus::Rect btnRect(sidebarW + 30, 195, 200, 50);
        if (m_isGameRunning && m_runningAppId == curApp.id) {
            // Stop button
            Gdiplus::SolidBrush stopBrush(Gdiplus::Color(255, 195, 45, 45));
            g.FillRectangle(&stopBrush, btnRect);

            Gdiplus::Font btnFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush btnTxt(Gdiplus::Color(255, 255, 255, 255));
            g.DrawString(L"■  ОСТАНОВИТЬ", -1, &btnFont, Gdiplus::PointF(static_cast<float>(btnRect.X + 28), static_cast<float>(btnRect.Y + 14)), &btnTxt);
        } else if (curApp.state == steam::core::AppState::Installed) {
            // Iconic Steam Green Play Button
            Gdiplus::LinearGradientBrush playBrush(btnRect,
                Gdiplus::Color(255, 117, 185, 24), // Bright Steam Play Green
                Gdiplus::Color(255, 87, 145, 15),
                Gdiplus::LinearGradientModeVertical
            );
            g.FillRectangle(&playBrush, btnRect);

            Gdiplus::Font btnFont(L"Segoe UI", 13.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush btnTxt(Gdiplus::Color(255, 255, 255, 255));
            g.DrawString(L"▶  ИГРАТЬ", -1, &btnFont, Gdiplus::PointF(static_cast<float>(btnRect.X + 50), static_cast<float>(btnRect.Y + 14)), &btnTxt);
        } else if (m_isDownloading) {
            // Downloading state
            Gdiplus::SolidBrush downBrush(Gdiplus::Color(255, 38, 70, 105));
            g.FillRectangle(&downBrush, btnRect);

            Gdiplus::Font btnFont(L"Segoe UI", 11.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
            Gdiplus::SolidBrush btnTxt(Gdiplus::Color(255, 102, 192, 244));
            g.DrawString(L"СКАЧИВАНИЕ...", -1, &btnFont, Gdiplus::PointF(static_cast<float>(btnRect.X + 35), static_cast<float>(btnRect.Y + 14)), &btnTxt);
        } else {
            // Install button
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

        // Stats Row next to button
        Gdiplus::Font statTitleFont(L"Segoe UI", 8.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
        Gdiplus::Font statValFont(L"Segoe UI", 10.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush statTitleTxt(Gdiplus::Color(255, 120, 135, 150));
        Gdiplus::SolidBrush statValTxt(Gdiplus::Color(255, 210, 220, 230));

        // Time played
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
            progSs << L"Загрузка чанков: " << std::fixed << std::setprecision(1)
                   << (m_downloadProgressFraction * 100.0f) << L"%  (" << m_downloadSpeedMbps << L" Mbps)";
            std::wstring ps = progSs.str();
            g.DrawString(ps.c_str(), -1, &statValFont, Gdiplus::PointF(static_cast<float>(sidebarW + 30), 280), &statValTxt);
        }

        // Section Cards below Wallpaper
        int cardsY = m_isDownloading ? 315 : 275;

        // Card 1: Game Description
        Gdiplus::Rect descRect(sidebarW + 30, cardsY, 460, 160);
        Gdiplus::SolidBrush cardBg(Gdiplus::Color(255, 26, 33, 44));
        g.FillRectangle(&cardBg, descRect);

        Gdiplus::Font cardHeadFont(L"Segoe UI", 11.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush cardHeadTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(L"ОБ ИГРЕ", -1, &cardHeadFont, Gdiplus::PointF(static_cast<float>(descRect.X + 16), static_cast<float>(descRect.Y + 12)), &cardHeadTxt);

        Gdiplus::Font bodyFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
        Gdiplus::SolidBrush bodyTxt(Gdiplus::Color(255, 175, 190, 205));

        if (curApp.id == 101) {
            std::wstring desc =
                L"Классическая текстовая ролевая игра по мотивам D&D 5-й редакции.\n"
                L"• Выбор рас (Человек, Эльф, Дварф, Полуорк, Драконорождённый...)\n"
                L"• 12 классов: Воин, Маг, Плут, Жрец, Варвар, Паладин и др.\n"
                L"• Инвентарь, экипировка, заклинания и зелья.\n"
                L"• Торговец Боб и интерактивная ASCII-арт энциклопедия.";
            g.DrawString(desc.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(descRect.X + 16), static_cast<float>(descRect.Y + 38)), &bodyTxt);
        } else {
            std::wstring desc = L"Полный цифровой дистрибутив с поддержкой синхронизации и модификаций.";
            g.DrawString(desc.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(descRect.X + 16), static_cast<float>(descRect.Y + 38)), &bodyTxt);
        }

        // Card 2: Achievements / Activity
        Gdiplus::Rect achRect(sidebarW + 510, cardsY, 280, 160);
        g.FillRectangle(&cardBg, achRect);
        g.DrawString(L"ДОСТИЖЕНИЯ", -1, &cardHeadFont, Gdiplus::PointF(static_cast<float>(achRect.X + 16), static_cast<float>(achRect.Y + 12)), &cardHeadTxt);

        std::wstring achDesc =
            L"Разблокировано: 4 из 12 (33%)\n\n"
            L"🏆 Первый бросок кубика\n"
            L"🏆 Друг торговца Боба\n"
            L"🏆 Мастер заклинаний\n"
            L"🏆 Победа над драконом";
        g.DrawString(achDesc.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(achRect.X + 16), static_cast<float>(achRect.Y + 38)), &bodyTxt);
    }
}

void SteamMainWindow::drawStoreTab(Gdiplus::Graphics& g, const RECT& r) {
    // Top Store Subnav: Просмотр, Рекомендации, Категории, Поиск (from screenshot 3)
    Gdiplus::SolidBrush storeNavBg(Gdiplus::Color(255, 30, 38, 48));
    g.FillRectangle(&storeNavBg, 20, 68, r.right - 40, 36);

    Gdiplus::Font navFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush navTxt(Gdiplus::Color(255, 210, 220, 230));
    g.DrawString(L"Просмотр ▼    Рекомендации ▼    Категории ▼    Прочее ▼", -1, &navFont, Gdiplus::PointF(35, 78), &navTxt);

    // Search input
    Gdiplus::SolidBrush searchBg(Gdiplus::Color(255, 45, 55, 68));
    g.FillRectangle(&searchBg, r.right - 340, 72, 200, 26);
    Gdiplus::SolidBrush searchTxt(Gdiplus::Color(255, 140, 150, 160));
    g.DrawString(L"Поиск по магазину...", -1, &navFont, Gdiplus::PointF(static_cast<float>(r.right - 330), 77), &searchTxt);

    // Wishlist Button
    Gdiplus::SolidBrush wishBg(Gdiplus::Color(255, 26, 159, 255));
    g.DrawString(L"★ Список желаемого (18)", -1, &navFont, Gdiplus::PointF(static_cast<float>(r.right - 130), 78), &wishBg);

    // FEATURED CAROUSEL (3 big cards matching screenshot 3)
    // American Truck Simulator (-75% 312 руб), READY OR NOT (-50% 1120 руб), BODYCAM (-20% 960 руб)
    struct FeaturedGame {
        std::wstring title;
        std::wstring genre;
        std::wstring discount;
        std::wstring oldPrice;
        std::wstring newPrice;
        Gdiplus::Color themeColor;
    };

    std::vector<FeaturedGame> games = {
        { L"AMERICAN TRUCK SIMULATOR", L"Симулятор вождения / Открытый мир", L"-75%", L"1249 руб.", L"312 руб.", Gdiplus::Color(255, 35, 60, 95) },
        { L"READY OR NOT", L"Тактический шутер / Спецназ SWAT", L"-50%", L"2240 руб.", L"1120 руб.", Gdiplus::Color(255, 120, 30, 30) },
        { L"BODYCAM", L"Фотореалистичный FPS / Мультиплеер", L"-20%", L"1200 руб.", L"960 руб.", Gdiplus::Color(255, 45, 55, 65) }
    };

    constexpr int cardW = 320;
    constexpr int cardH = 240;
    const int startX = (r.right - (cardW * 3 + 40)) / 2;
    int currentX = startX;

    Gdiplus::Font gameHead(L"Segoe UI", 11.5f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font genreFont(L"Segoe UI", 8.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::Font discFont(L"Segoe UI", 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font priceOldFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleStrikeout, Gdiplus::UnitPoint);
    Gdiplus::Font priceNewFont(L"Segoe UI", 11.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);

    for (std::size_t i = 0; i < games.size(); ++i) {
        const auto& fg = games[i];

        // Card Container
        Gdiplus::Rect cRect(currentX, 120, cardW, cardH);
        Gdiplus::LinearGradientBrush cardBrush(cRect, fg.themeColor, Gdiplus::Color(255, 20, 26, 34), Gdiplus::LinearGradientModeVertical);
        g.FillRectangle(&cardBrush, cRect);

        // Card Title & genre
        Gdiplus::SolidBrush whiteTxt(Gdiplus::Color(255, 255, 255, 255));
        Gdiplus::SolidBrush subTxt(Gdiplus::Color(255, 180, 190, 200));
        g.DrawString(fg.title.c_str(), -1, &gameHead, Gdiplus::PointF(static_cast<float>(currentX + 14), 135), &whiteTxt);
        g.DrawString(fg.genre.c_str(), -1, &genreFont, Gdiplus::PointF(static_cast<float>(currentX + 14), 160), &subTxt);

        // Discount Tag (-75% Green badge)
        Gdiplus::Rect discRect(currentX + 14, 305, 55, 26);
        Gdiplus::SolidBrush discBadge(Gdiplus::Color(255, 164, 208, 7)); // Steam Green
        g.FillRectangle(&discBadge, discRect);

        Gdiplus::SolidBrush discTxt(Gdiplus::Color(255, 0, 0, 0));
        g.DrawString(fg.discount.c_str(), -1, &discFont, Gdiplus::PointF(static_cast<float>(discRect.X + 6), static_cast<float>(discRect.Y + 3)), &discTxt);

        // Price tags
        Gdiplus::SolidBrush oldPriceTxt(Gdiplus::Color(255, 120, 130, 140));
        Gdiplus::SolidBrush newPriceTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(fg.oldPrice.c_str(), -1, &priceOldFont, Gdiplus::PointF(static_cast<float>(currentX + 78), 302), &oldPriceTxt);
        g.DrawString(fg.newPrice.c_str(), -1, &priceNewFont, Gdiplus::PointF(static_cast<float>(currentX + 78), 316), &newPriceTxt);

        currentX += cardW + 20;
    }

    // SECTION 2: Крупные скидки на избранные игры (Warhammer Mechanicus, Jagged Alliance 3, PlateUp!)
    int sec2Y = 385;
    Gdiplus::Font secTitle(L"Segoe UI", 13.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush secTxt(Gdiplus::Color(255, 255, 255, 255));
    g.DrawString(L"Крупные скидки на избранные игры", -1, &secTitle, Gdiplus::PointF(static_cast<float>(startX), static_cast<float>(sec2Y)), &secTxt);

    std::vector<FeaturedGame> discounts = {
        { L"Warhammer 40,000: Mechanicus", L"Пошаговая тактика", L"-95%", L"1100 руб.", L"55 руб.", Gdiplus::Color(255, 30, 50, 40) },
        { L"Jagged Alliance 3", L"Тактическая RPG", L"-90%", L"1999 руб.", L"199 руб.", Gdiplus::Color(255, 60, 45, 30) },
        { L"PlateUp!", L"Кооперативный рогалик", L"-90%", L"650 руб.", L"65 руб.", Gdiplus::Color(255, 50, 40, 60) }
    };

    currentX = startX;
    for (const auto& dg : discounts) {
        Gdiplus::Rect cRect(currentX, sec2Y + 30, cardW, 140);
        Gdiplus::LinearGradientBrush cardBrush(cRect, dg.themeColor, Gdiplus::Color(255, 20, 24, 30), Gdiplus::LinearGradientModeVertical);
        g.FillRectangle(&cardBrush, cRect);

        Gdiplus::SolidBrush whiteTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(dg.title.c_str(), -1, &gameHead, Gdiplus::PointF(static_cast<float>(currentX + 12), static_cast<float>(sec2Y + 40)), &whiteTxt);

        // Discount Tag
        Gdiplus::Rect discRect(currentX + 12, sec2Y + 130, 52, 24);
        Gdiplus::SolidBrush discBadge(Gdiplus::Color(255, 164, 208, 7));
        g.FillRectangle(&discBadge, discRect);
        Gdiplus::SolidBrush discTxt(Gdiplus::Color(255, 0, 0, 0));
        g.DrawString(dg.discount.c_str(), -1, &discFont, Gdiplus::PointF(static_cast<float>(discRect.X + 4), static_cast<float>(discRect.Y + 2)), &discTxt);

        // Price tags
        Gdiplus::SolidBrush oldPriceTxt(Gdiplus::Color(255, 120, 130, 140));
        Gdiplus::SolidBrush newPriceTxt(Gdiplus::Color(255, 255, 255, 255));
        g.DrawString(dg.oldPrice.c_str(), -1, &priceOldFont, Gdiplus::PointF(static_cast<float>(currentX + 72), static_cast<float>(sec2Y + 128)), &oldPriceTxt);
        g.DrawString(dg.newPrice.c_str(), -1, &priceNewFont, Gdiplus::PointF(static_cast<float>(currentX + 72), static_cast<float>(sec2Y + 140)), &newPriceTxt);

        currentX += cardW + 20;
    }
}

void SteamMainWindow::drawCommunityTab(Gdiplus::Graphics& g, const RECT& r) {
    Gdiplus::Font headFont(L"Segoe UI", 16.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font bodyFont(L"Segoe UI", 9.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush whiteTxt(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush subTxt(Gdiplus::Color(255, 170, 185, 200));

    g.DrawString(L"Сообщество Steam — Центр активности", -1, &headFont, Gdiplus::PointF(40, 80), &whiteTxt);

    // Discussion Card 1: D&D Console RPG
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

    // Workshop Card 2
    Gdiplus::Rect card2(40, 255, r.right - 80, 110);
    g.FillRectangle(&cardBg, card2);

    g.DrawString(L"🛠 Мастерская Steam — Популярные модификации", -1, &cardTitle, Gdiplus::PointF(static_cast<float>(card2.X + 16), static_cast<float>(card2.Y + 14)), &whiteTxt);

    std::wstring text2 =
        L"Мод: «Расширенный бестиарий и 15 легендарных артефактов» (v1.4)\n"
        L"Рейтинг: ★★★★★ (1,290 подписчиков)\n"
        L"Добавляет уникальных монстров в пещеры и новый инвентарь для торговца Боба.";
    g.DrawString(text2.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(card2.X + 16), static_cast<float>(card2.Y + 42)), &subTxt);

    // Screenshot Card 3
    Gdiplus::Rect card3(40, 385, r.right - 80, 110);
    g.FillRectangle(&cardBg, card3);

    g.DrawString(L"📷 Галерея скриншотов и ASCII-Арта", -1, &cardTitle, Gdiplus::PointF(static_cast<float>(card3.X + 16), static_cast<float>(card3.Y + 14)), &whiteTxt);

    std::wstring text3 =
        L"Пользователь: DragonKnight  •  «ASCII-Арт финального босса в полном разрешении!»\n"
        L"Понравилось 418 игрокам  •  12 наград сообщества.";
    g.DrawString(text3.c_str(), -1, &bodyFont, Gdiplus::PointF(static_cast<float>(card3.X + 16), static_cast<float>(card3.Y + 42)), &subTxt);
}

void SteamMainWindow::drawBottomBar(Gdiplus::Graphics& g, const RECT& r) {
    // Bottom Bar matching screenshot 2/3
    Gdiplus::SolidBrush botBg(Gdiplus::Color(255, 23, 26, 33));
    g.FillRectangle(&botBg, 0, r.bottom - 32, r.right, 32);

    Gdiplus::Font botFont(L"Segoe UI", 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::SolidBrush botTxt(Gdiplus::Color(255, 140, 150, 160));

    // + Добавить игру
    g.DrawString(L"➕ Добавить игру", -1, &botFont, Gdiplus::PointF(16, static_cast<float>(r.bottom - 24)), &botTxt);

    // Управление загрузками
    std::wstring downText = L"⬇ Управление загрузками";
    if (m_isDownloading) {
        downText += L" (1 активно)";
    }
    g.DrawString(downText.c_str(), -1, &botFont, Gdiplus::PointF(static_cast<float>(r.right / 2 - 80), static_cast<float>(r.bottom - 24)), &botTxt);

    // Друзья и чат
    g.DrawString(L"👥 Друзья и чат (3)", -1, &botFont, Gdiplus::PointF(static_cast<float>(r.right - 140), static_cast<float>(r.bottom - 24)), &botTxt);
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
