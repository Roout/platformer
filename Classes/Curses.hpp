#ifndef CURSES_HPP
#define CURSES_HPP

class Unit;

namespace Curses {

    static constexpr float UNLIMITED { -100.f };

    enum class CurseType {
        DPS,
        SLOW,
        COUNT
    };

    class Curse {
    public:
        virtual bool CanEffect() const noexcept = 0;

        virtual bool IsExpired() const noexcept = 0;

        virtual void Update(const float dt) noexcept = 0;

        virtual void EffectUnit(Unit * const) noexcept = 0;
    };

    class DPS final : public Curse {
    public:

        DPS(float damagePerSecond, float duration);

        void Update(const float dt) noexcept override;

        void EffectUnit(Unit * const unit) noexcept override;

        [[nodiscard]] bool CanEffect() const noexcept override {
            return m_cooldown <= 0.f;
        }

        [[nodiscard]] bool IsExpired() const noexcept override {
            return m_duration != UNLIMITED && m_duration <= 0.f;
        }

    private:

        const float m_damage { 0.f };
        
        float m_duration { UNLIMITED };
        float m_cooldown { UNLIMITED };
    };

    /**
     * Some template magic.
     * Help to get Type by enum value.
     */
    template<CurseType curse_type>
    struct get_curse_type final {
        CurseType value { curse_type };
    };

    template<>
    struct get_curse_type<CurseType::DPS> {
        CurseType value { CurseType::DPS };

        using identity = DPS;
    };
};

#endif // CURSES_HPP