cmake_minimum_required(VERSION 3.22)
project(SteamClone
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd / IDE tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Directory layout
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Common warnings interface
add_library(project_warnings INTERFACE)
target_compile_features(project_warnings INTERFACE cxx_std_20)

if(MSVC)
    target_compile_options(project_warnings INTERFACE
        /W4 /permissive- /utf-8
    )
else()
    target_compile_options(project_warnings INTERFACE
        -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
    )
endif()

# Core engine and domain models
add_subdirectory(src/core)
add_subdirectory(src/downloader)
add_subdirectory(src/launcher)
add_subdirectory(src/storage)
add_subdirectory(src/ui)

# Client Application
add_executable(steam_client src/main.cpp)
target_link_libraries(steam_client PRIVATE
    project_warnings
    steam_downloader
    steam_launcher
    steam_storage
    steam_ui
    steam_core
)

if(WIN32)
    target_link_libraries(steam_client PRIVATE winhttp gdi32 user32 shell32 gdiplus msimg32)
endif()
