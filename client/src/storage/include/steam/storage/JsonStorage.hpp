#pragma once

#include "steam/storage/ILocalStorage.hpp"
#include <filesystem>
#include <map>
#include <mutex>

namespace steam::storage {

class JsonStorage final : public ILocalStorage {
public:
    explicit JsonStorage(std::filesystem::path storageFilePath);
    ~JsonStorage() override = default;

    bool load() override;
    bool save() override;

    void saveAppManifest(const steam::core::AppManifest& manifest) override;
    [[nodiscard]] std::optional<steam::core::AppManifest> getAppManifest(steam::core::AppId appId) const override;
    [[nodiscard]] virtual std::vector<steam::core::AppManifest> getAllApps() const override;
    void removeApp(steam::core::AppId appId) override;

    void addPlaytime(steam::core::AppId appId, std::uint64_t seconds) override;

private:
    std::filesystem::path m_filePath;
    mutable std::mutex m_mutex;
    std::map<steam::core::AppId, steam::core::AppManifest> m_manifests;
};

} // namespace steam::storage
