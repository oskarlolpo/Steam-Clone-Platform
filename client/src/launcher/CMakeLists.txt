add_library(steam_launcher STATIC
    src/ProcessLauncher.cpp
)

target_include_directories(steam_launcher PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(steam_launcher PUBLIC
    steam_core
    project_warnings
)
