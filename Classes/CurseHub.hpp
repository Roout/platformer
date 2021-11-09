#ifndef CURSE_HUB_HPP
#define CURSE_HUB_HPP

#include "Curses.hpp"
#include "Utils.hpp"
#include <memory>
#include <array>
#include <limits>

namespace curses {
    /**
     * The curses manager which belongs to unit. 
     * Help to manage curses applied to the unit.
     * 
     * @note
     * Unit can have only one type of curse at one time. 
     * Adding new one of the same type (if it's already exist) will break a logic.
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

        /**
         * Linear complexity: perform a search for the first free space 
         * in the curse holder array
         */
        template<CurseClass type, class ...Args>
        void AddCurse(size_t trapId, Args&&... args) {
            using identity = typename get_curse<type>::identity;
            
            for (auto& curse: m_curses) {
                if (!curse) {
                    curse = std::make_unique<identity>(trapId, std::forward<Args>(args)...);
                    break;
                }
            }
        }

        void RemoveCurse(size_t id) noexcept {
            for (auto& curse: m_curses) {
                if (curse && curse->GetId() == id) {
                    curse.reset();
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