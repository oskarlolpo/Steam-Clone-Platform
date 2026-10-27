add_library(steam_storage STATIC
    src/JsonStorage.cpp
)

target_include_directories(steam_storage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(steam_storage PUBLIC
    steam_core
    project_warnings
)
