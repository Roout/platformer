#ifndef UNIT_VIEW_HPP
#define UNIT_VIEW_HPP

#include "cocos2d.h"
#include "Unit.hpp"

class UnitView final : public cocos2d::DrawNode {
public:
    static UnitView * create(Unit* const model);

    [[nodiscard]] bool init() override;

    void update([[maybe_unused]] float dt) override;

    /**
     * Functions used to modify unit.
     * Used by callbacks: onContactBegin, OnContactSeparate because it's easy to 
     * retrieve this drawable node (view) than pass unit's model. 
     */
    void SetContactWithGround(bool) noexcept;

    template<Curses::CurseType type, class ...Args>
    void AddCurse(size_t id, Args&&... args) noexcept {
        return m_model->AddCurse<type>(id, std::forward<Args>(args)...);
    }

    void RemoveCurse(size_t id) noexcept {
        m_model->RemoveCurse(id);
    }
    // end of functions which modify unit
private:
    UnitView(Unit* const model);

    void UpdateAnimation();

    void FlipX(const Unit::Side);
private:

    Unit * const m_model { nullptr };

    /**
     * Data from the previous unit state.
     * Used to monitor the changes of the unit state so that view will also be changed 
     * with animation and other stuff.
     */
    struct LastUpdate {
        Unit::Side   direction { Unit::Side::left };
        Unit::State     state { Unit::State::idle };
    } m_lastState {};
};

#endif // UNIT_VIEW_HPP