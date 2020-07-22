#ifndef CURSE_HUB_HPP
#define CURSE_HUB_HPP

#include "Curses.hpp"
#include "Utils.hpp"
#include <memory>
#include <array>
#include <limits>

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
        /**
         * This 'id' is used to mark curses which came not from traps.
         */
        static constexpr size_t ignored { std::numeric_limits<size_t>::max() };

        using CursePointer = std::unique_ptr<Curse>;

        CurseHub(Unit * const);

        void Update(const float dt);

        template<CurseType type, class ...Args>
        void AddCurse(size_t trapId, Args&&... args) noexcept {
            using identity = typename get_curse_type<type>::identity;

            for(size_t i = 0; i < MAX_SIZE; i++) {
                if(!m_curses[i]) {
                    m_curses[i] = std::make_unique<identity>( trapId, std::forward<Args>(args)...);
                    break;
                }
            }
        }

        void RemoveCurse(size_t id) noexcept {
            for(size_t i = 0; i < MAX_SIZE; i++) {
                if( m_curses[i] && m_curses[i]->GetId() == id ) {
                    m_curses[i].reset();
                }
            }
        }

    private:
        static constexpr size_t MAX_SIZE { 16 };

        Unit * const m_unit { nullptr };

        std::array<CursePointer, MAX_SIZE> m_curses;
    };

}

#endif // CURSE_HUB_HPP