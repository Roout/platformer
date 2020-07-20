#ifndef CURSE_HUB_HPP
#define CURSE_HUB_HPP

#include "Curses.hpp"
#include "Core.hpp"
#include <memory>
#include <array>

class Unit;

namespace Curses {
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

}

#endif // CURSE_HUB_HPP