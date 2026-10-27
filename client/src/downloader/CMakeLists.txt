add_library(steam_downloader STATIC
    src/DownloadManager.cpp
    src/ArchiveExtractor.cpp
)

target_include_directories(steam_downloader PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(steam_downloader PUBLIC 
    steam_core
    project_warnings
)

if(WIN32)
    target_link_libraries(steam_downloader PRIVATE winhttp)
endif()
