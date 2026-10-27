#pragma once

#include "steam/core/Types.hpp"
#include <filesystem>
#include <string>

namespace steam::core {

struct AppManifest {
    AppId id{0};
    std::string title;
    std::string version{"1.0.0"};
    std::uint64_t installSize{0};
    std::filesystem::path installDirectory;
    std::string executableRelativePath;
    std::string downloadUrl;
    std::string sha256Checksum;
    AppState state{AppState::NotInstalled};
    std::uint64_t totalPlaytimeSeconds{0};
};

} // namespace steam::core
