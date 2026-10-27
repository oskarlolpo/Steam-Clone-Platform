#pragma once

#include "steam/core/AppManifest.hpp"
#include <optional>
#include <vector>

namespace steam::storage {

class ILocalStorage {
public:
    virtual ~ILocalStorage() = default;

    virtual bool load() = 0;
    virtual bool save() = 0;

    virtual void saveAppManifest(const steam::core::AppManifest& manifest) = 0;
    [[nodiscard]] virtual std::optional<steam::core::AppManifest> getAppManifest(steam::core::AppId appId) const = 0;
    [[nodiscard]] virtual std::vector<steam::core::AppManifest> getAllApps() const = 0;
    virtual void removeApp(steam::core::AppId appId) = 0;

    virtual void addPlaytime(steam::core::AppId appId, std::uint64_t seconds) = 0;
};

} // namespace steam::storage
