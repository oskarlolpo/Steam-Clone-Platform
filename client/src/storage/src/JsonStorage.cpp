#include "steam/storage/JsonStorage.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace steam::storage {

JsonStorage::JsonStorage(std::filesystem::path storageFilePath)
    : m_filePath(std::move(storageFilePath)) {}

bool JsonStorage::load() {
    std::scoped_lock lock(m_mutex);
    if (!std::filesystem::exists(m_filePath)) {
        return false;
    }

    std::ifstream file(m_filePath);
    if (!file.is_open()) {
        return false;
    }

    // Simple line-based/JSON-key parser for reliable persistence without 3rd-party bloat
    std::string line;
    steam::core::AppManifest current;
    bool inApp = false;

    while (std::getline(file, line)) {
        auto trim = [](std::string s) {
            auto start = s.find_first_not_of(" \t\r\n\",");
            if (start == std::string::npos) return std::string{};
            auto end = s.find_last_not_of(" \t\r\n\",");
            return s.substr(start, end - start + 1);
        };

        if (line.find("\"app\": {") != std::string::npos || line.find("\"id\":") != std::string::npos) {
            inApp = true;
        }

        auto parseKeyValue = [&](const std::string& key) -> std::string {
            auto pos = line.find("\"" + key + "\":");
            if (pos != std::string::npos) {
                auto valStart = line.find(':', pos) + 1;
                return trim(line.substr(valStart));
            }
            return {};
        };

        std::string parsedVal;
        if (parsedVal = parseKeyValue("id"); !parsedVal.empty()) {
            current.id = std::stoull(parsedVal);
        } else if (parsedVal = parseKeyValue("title"); !parsedVal.empty()) {
            current.title = parsedVal;
        } else if (parsedVal = parseKeyValue("version"); !parsedVal.empty()) {
            current.version = parsedVal;
        } else if (parsedVal = parseKeyValue("installSize"); !parsedVal.empty()) {
            current.installSize = std::stoull(parsedVal);
        } else if (parsedVal = parseKeyValue("installDirectory"); !parsedVal.empty()) {
            current.installDirectory = parsedVal;
        } else if (parsedVal = parseKeyValue("executableRelativePath"); !parsedVal.empty()) {
            current.executableRelativePath = parsedVal;
        } else if (parsedVal = parseKeyValue("downloadUrl"); !parsedVal.empty()) {
            current.downloadUrl = parsedVal;
        } else if (parsedVal = parseKeyValue("sha256Checksum"); !parsedVal.empty()) {
            current.sha256Checksum = parsedVal;
        } else if (parsedVal = parseKeyValue("state"); !parsedVal.empty()) {
            current.state = static_cast<steam::core::AppState>(std::stoul(parsedVal));
        } else if (parsedVal = parseKeyValue("totalPlaytimeSeconds"); !parsedVal.empty()) {
            current.totalPlaytimeSeconds = std::stoull(parsedVal);
        }

        if (line.find("}") != std::string::npos && inApp && current.id != 0) {
            m_manifests[current.id] = current;
            current = steam::core::AppManifest{};
            inApp = false;
        }
    }

    return true;
}

bool JsonStorage::save() {
    std::scoped_lock lock(m_mutex);

    if (m_filePath.has_parent_path()) {
        std::filesystem::create_directories(m_filePath.parent_path());
    }

    // Atomic write pattern: write to tmp then rename
    const auto tempPath = m_filePath.string() + ".tmp";
    std::ofstream file(tempPath);
    if (!file.is_open()) {
        return false;
    }

    file << "{\n  \"apps\": [\n";
    bool first = true;
    for (const auto& [id, m] : m_manifests) {
        if (!first) {
            file << ",\n";
        }
        first = false;

        file << "    {\n"
             << "      \"id\": " << m.id << ",\n"
             << "      \"title\": \"" << m.title << "\",\n"
             << "      \"version\": \"" << m.version << "\",\n"
             << "      \"installSize\": " << m.installSize << ",\n"
             << "      \"installDirectory\": \"" << m.installDirectory.string() << "\",\n"
             << "      \"executableRelativePath\": \"" << m.executableRelativePath << "\",\n"
             << "      \"downloadUrl\": \"" << m.downloadUrl << "\",\n"
             << "      \"sha256Checksum\": \"" << m.sha256Checksum << "\",\n"
             << "      \"state\": " << static_cast<int>(m.state) << ",\n"
             << "      \"totalPlaytimeSeconds\": " << m.totalPlaytimeSeconds << "\n"
             << "    }";
    }
    file << "\n  ]\n}\n";
    file.close();

    std::error_code ec;
    std::filesystem::rename(tempPath, m_filePath, ec);
    if (ec) {
        // Fallback for platforms with cross-device or open handle rename constraints
        std::filesystem::copy_file(tempPath, m_filePath, std::filesystem::copy_options::overwrite_existing, ec);
        std::filesystem::remove(tempPath, ec);
    }

    return true;
}

void JsonStorage::saveAppManifest(const steam::core::AppManifest& manifest) {
    std::scoped_lock lock(m_mutex);
    m_manifests[manifest.id] = manifest;
}

std::optional<steam::core::AppManifest> JsonStorage::getAppManifest(steam::core::AppId appId) const {
    std::scoped_lock lock(m_mutex);
    if (auto it = m_manifests.find(appId); it != m_manifests.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<steam::core::AppManifest> JsonStorage::getAllApps() const {
    std::scoped_lock lock(m_mutex);
    std::vector<steam::core::AppManifest> result;
    result.reserve(m_manifests.size());
    for (const auto& [id, manifest] : m_manifests) {
        result.push_back(manifest);
    }
    return result;
}

void JsonStorage::removeApp(steam::core::AppId appId) {
    std::scoped_lock lock(m_mutex);
    m_manifests.erase(appId);
}

void JsonStorage::addPlaytime(steam::core::AppId appId, std::uint64_t seconds) {
    std::scoped_lock lock(m_mutex);
    if (auto it = m_manifests.find(appId); it != m_manifests.end()) {
        it->second.totalPlaytimeSeconds += seconds;
    }
}

} // namespace steam::storage
