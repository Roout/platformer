#ifndef CURSES_HPP
#define CURSES_HPP

class Unit;

namespace curses {

    static constexpr float UNLIMITED { -100.f };

    enum class CurseClass {
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
            return m_cooldown <= 0.f && !IsExpired();
        }

        [[nodiscard]] bool IsExpired() const noexcept override {
            return m_duration != UNLIMITED && m_duration <= 0.f;
        }

    protected:

        const float m_damage { 0.f };
        
        float m_duration { UNLIMITED };
        float m_cooldown { UNLIMITED };
    };
    
    class Instant final : public DPS {
    public:
        /**
         * Duration big enough for curse to deal some damage once
         * and expire before coldown turns 0
         */
        static constexpr float DURATION { 0.5f };

        Instant(size_t id, float damage) : 
            DPS{ id, damage, DURATION }
        {};

    };

    /**
     * Some template magic.
     * Help to get Type by enum value.
     */
    template<CurseClass curse_type>
    struct get_curse final {};

    template<>
    struct get_curse<CurseClass::DPS> {
        using identity = DPS;
    };

    template<>
    struct get_curse<CurseClass::INSTANT> {
        using identity = Instant;
    };

}; // namespace curses 

#endif // CURSES_HPP