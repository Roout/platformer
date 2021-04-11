#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

#include "Utils.hpp"

#include <array>

namespace settings {

class DebugMode final {
public:
    enum class OptionKind {
        kPhysics
        , kInvicible
        , kState
        , COUNT
    };

    ~DebugMode() = default;

    DebugMode(const DebugMode&) = delete;
    DebugMode& operator=(const DebugMode&) = delete;
    DebugMode(DebugMode&&) = delete;
    DebugMode& operator=(DebugMode&&) = delete;

    bool IsEnabled(OptionKind opt) const noexcept {
        return m_options[Utils::EnumCast(opt)];
    }

    void Enable(OptionKind opt) noexcept {
        m_options[Utils::EnumCast(opt)] = true;
    }

    void Disable(OptionKind opt) noexcept {
        m_options[Utils::EnumCast(opt)] = false;
    }

    void Toggle(OptionKind opt) noexcept {
        bool isOn = m_options[Utils::EnumCast(opt)];
        m_options[Utils::EnumCast(opt)] = isOn? false: true;
    }

    static DebugMode& GetInstance() noexcept {
        static DebugMode mode{};
        return mode;
    }

private:
    std::array<bool, Utils::EnumSize<OptionKind>()> m_options;

    DebugMode() {
        m_options.fill(false);
    }

};

} // namespace settings

#endif // SETTINGS_HPP_