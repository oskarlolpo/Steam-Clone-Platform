#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace steam::core {

class Sha256 {
public:
    Sha256();
    void update(const std::uint8_t* data, std::size_t length);
    void update(std::string_view data);
    [[nodiscard]] std::string finalHex();
    void reset();

    // Utility functions
    [[nodiscard]] static std::string hashString(std::string_view input);
    [[nodiscard]] static std::string hashFile(const std::filesystem::path& filePath);

private:
    void transform(const std::uint8_t* chunk);

    std::array<std::uint32_t, 8> m_state{};
    std::uint64_t m_bitCount{0};
    std::array<std::uint8_t, 64> m_buffer{};
};

} // namespace steam::core
