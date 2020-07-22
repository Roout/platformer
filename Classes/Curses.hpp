#ifndef CURSES_HPP
#define CURSES_HPP

class Unit;

namespace Curses {

    static constexpr float UNLIMITED { -100.f };

    enum class CurseType {
        DPS,
        INSTANT,
        SLOW,
        COUNT
    };

    class Curse {
    public:
        Curse(size_t trapId): 
            m_trapId { trapId } 
        {}

        virtual bool CanEffect() const noexcept = 0;

        virtual bool IsExpired() const noexcept = 0;

        virtual void Update(const float dt) noexcept = 0;

        virtual void EffectUnit(Unit * const) noexcept = 0;

        size_t GetId() const noexcept {
            return m_trapId;
        }

        bool operator== (const Curse& curse) const noexcept {
            return curse.m_trapId == m_trapId;
        }

    private:
        const size_t m_trapId { 0 };
    };

    class DPS : public Curse {
    public:

        DPS(size_t id, float damagePerSecond, float duration);

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
    
    class Instant final : public DPS {
    public:

        Instant(size_t id, float damage) : 
            DPS{ id, damage, 1.f }
        {};

    };

    /**
     * Some template magic.
     * Help to get Type by enum value.
     */
    template<CurseType curse_type>
    struct get_curse_type final {};

    template<>
    struct get_curse_type<CurseType::DPS> {
        using identity = DPS;
    };

    template<>
    struct get_curse_type<CurseType::INSTANT> {
        using identity = Instant;
    };
};

#endif // CURSES_HPP