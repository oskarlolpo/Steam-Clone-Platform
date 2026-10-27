#pragma once

#include "steam/core/Types.hpp"
#include <chrono>
#include <string>

namespace steam::core {

class UserSession {
public:
    UserSession() = default;
    UserSession(UserId id, std::string username, std::string sessionToken)
        : m_userId(id), m_username(std::move(username)), m_sessionToken(std::move(sessionToken)),
          m_loginTime(std::chrono::system_clock::now()) {}

    [[nodiscard]] UserId getUserId() const noexcept { return m_userId; }
    [[nodiscard]] const std::string& getUsername() const noexcept { return m_username; }
    [[nodiscard]] const std::string& getSessionToken() const noexcept { return m_sessionToken; }
    [[nodiscard]] bool isAuthenticated() const noexcept { return !m_sessionToken.empty(); }
    [[nodiscard]] std::chrono::system_clock::time_point getLoginTime() const noexcept { return m_loginTime; }

    void invalidate() noexcept {
        m_sessionToken.clear();
    }

private:
    UserId m_userId{0};
    std::string m_username;
    std::string m_sessionToken;
    std::chrono::system_clock::time_point m_loginTime{};
};

} // namespace steam::core
