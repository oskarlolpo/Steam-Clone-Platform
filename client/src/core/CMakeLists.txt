add_library(steam_core STATIC
    src/Sha256.cpp
)

target_include_directories(steam_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(steam_core PUBLIC project_warnings)
