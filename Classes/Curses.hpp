#ifndef CURSES_HPP
#define CURSES_HPP

#include <array>
#include <memory>
#include "Core.hpp"

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

    /**
     * The curses manager which belongs to unit. 
     * Help to manage curses applied to the unit.
     * 
     * @note
     * Unit can have only one type of curse at one time. 
     * Adding new one of the same type (if it's already exist) is impossible.
     */
    class CurseHub final {
    public:
        using CursePointer = std::unique_ptr<Curse>;

        CurseHub(Unit * const);

        void Update(const float dt);

        template<CurseType type, class ...Args>
        void AddCurse(Args&&... args) noexcept {
            using identity = typename get_curse_type<type>::identity;

            if(!m_curses[core::EnumCast(type)]) {
                m_curses[core::EnumCast(type)] = std::make_unique<identity>(std::forward<Args>(args)...);
            }
        }

        template<CurseType type>
        void RemoveCurse() noexcept {
            m_curses[core::EnumCast(type)].reset();
        }

    private:
        Unit * const m_unit { nullptr };

        std::array<CursePointer, core::EnumSize<CurseType>()> m_curses;
    };      

};

#endif // CURSES_HPP