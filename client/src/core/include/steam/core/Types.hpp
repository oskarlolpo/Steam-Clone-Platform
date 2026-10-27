#pragma once

#include <cstdint>
#include <string>

namespace steam::core {

using AppId = std::uint64_t;
using UserId = std::uint64_t;

enum class AppState : std::uint8_t {
    NotInstalled,
    Queued,
    Downloading,
    Validating,
    Installed,
    Running,
    Error
};

} // namespace steam::core
