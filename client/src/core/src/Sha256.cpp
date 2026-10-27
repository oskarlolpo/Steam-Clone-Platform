#include "steam/core/Sha256.hpp"
#include <bit>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace steam::core {

namespace {

constexpr std::array<std::uint32_t, 64> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline std::uint32_t rotr(std::uint32_t x, std::uint32_t n) noexcept {
    return std::rotr(x, n);
}

inline std::uint32_t ch(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept {
    return (x & y) ^ (~x & z);
}

inline std::uint32_t maj(std::uint32_t x, std::uint32_t y, std::uint32_t z) noexcept {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline std::uint32_t sigma0(std::uint32_t x) noexcept {
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
}

inline std::uint32_t sigma1(std::uint32_t x) noexcept {
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
}

inline std::uint32_t gamma0(std::uint32_t x) noexcept {
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
}

inline std::uint32_t gamma1(std::uint32_t x) noexcept {
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
}

} // namespace

Sha256::Sha256() {
    reset();
}

void Sha256::reset() {
    m_state = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    m_bitCount = 0;
    m_buffer.fill(0);
}

void Sha256::transform(const std::uint8_t* chunk) {
    std::array<std::uint32_t, 64> w{};
    for (std::size_t i = 0; i < 16; ++i) {
        w[i] = (static_cast<std::uint32_t>(chunk[i * 4]) << 24) |
               (static_cast<std::uint32_t>(chunk[i * 4 + 1]) << 16) |
               (static_cast<std::uint32_t>(chunk[i * 4 + 2]) << 8) |
               (static_cast<std::uint32_t>(chunk[i * 4 + 3]));
    }
    for (std::size_t i = 16; i < 64; ++i) {
        w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
    }

    std::uint32_t a = m_state[0];
    std::uint32_t b = m_state[1];
    std::uint32_t c = m_state[2];
    std::uint32_t d = m_state[3];
    std::uint32_t e = m_state[4];
    std::uint32_t f = m_state[5];
    std::uint32_t g = m_state[6];
    std::uint32_t h = m_state[7];

    for (std::size_t i = 0; i < 64; ++i) {
        const std::uint32_t t1 = h + sigma1(e) + ch(e, f, g) + K[i] + w[i];
        const std::uint32_t t2 = sigma0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
    m_state[4] += e;
    m_state[5] += f;
    m_state[6] += g;
    m_state[7] += h;
}

void Sha256::update(const std::uint8_t* data, std::size_t length) {
    for (std::size_t i = 0; i < length; ++i) {
        m_buffer[(m_bitCount / 8) % 64] = data[i];
        m_bitCount += 8;
        if ((m_bitCount / 8) % 64 == 0) {
            transform(m_buffer.data());
        }
    }
}

void Sha256::update(std::string_view data) {
    update(reinterpret_cast<const std::uint8_t*>(data.data()), data.size());
}

std::string Sha256::finalHex() {
    const std::uint64_t totalBits = m_bitCount;
    update(reinterpret_cast<const std::uint8_t*>("\x80"), 1);
    while ((m_bitCount / 8) % 64 != 56) {
        update(reinterpret_cast<const std::uint8_t*>("\0"), 1);
    }

    for (int i = 7; i >= 0; --i) {
        const std::uint8_t byte = static_cast<std::uint8_t>((totalBits >> (i * 8)) & 0xFF);
        update(&byte, 1);
    }

    std::ostringstream ss;
    for (const auto val : m_state) {
        ss << std::hex << std::setw(8) << std::setfill('0') << val;
    }
    return ss.str();
}

std::string Sha256::hashString(std::string_view input) {
    Sha256 hasher;
    hasher.update(input);
    return hasher.finalHex();
}

std::string Sha256::hashFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    Sha256 hasher;
    std::array<std::uint8_t, 65536> buffer{};
    while (file.read(reinterpret_cast<char*>(buffer.data()), buffer.size()) || file.gcount() > 0) {
        hasher.update(buffer.data(), static_cast<std::size_t>(file.gcount()));
    }
    return hasher.finalHex();
}

} // namespace steam::core
