#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "cocos2d.h"

#include "Unit.hpp"
#include <string>

class Influence;

namespace Enemies {

/**
 * States bots can possibly have depending on their type 
 */
enum class State {
    UNDEFINED,
    
    // movement:
    PATROL,
    PURSUIT,
    WALK,
    DASH,
    
    IDLE,
    PREPARE_ATTACK,
    ATTACK,
    DEAD,

    // boss special states
    FIREBALL_ATTACK,
    FIRECLOUD_ATTACK,
    SWEEP_ATTACK,
    BASIC_ATTACK,
    BASIC_WALK,

    // cloud states
    INIT,
    EARLY,
    MID,
    LATE,

    COUNT
};

std::string GetStateName(State state);

class Bot : public Unit {
public:

    [[nodiscard]] bool init() override;
    
    inline size_t GetId() const noexcept;

    void AttachInfluenceArea(const cocos2d::Rect& area);

    virtual void OnEnemyIntrusion() = 0;

    virtual void OnEnemyLeave() = 0;

protected:

    Bot(size_t id, const std::string& dragonBonesName);
    
    void UpdateDebugLabel() noexcept override;

    virtual void TryAttack();

    virtual bool NeedAttack() const noexcept;

    /// Properties:
protected:

    State m_currentState  { State::UNDEFINED };

    State m_previousState { State::UNDEFINED };

    bool m_detectEnemy { false };

    Influence * m_influence { nullptr };

private:
    const size_t m_id { 0 };

};

inline size_t Bot::GetId() const noexcept {
    return m_id;
}

} // namespace Enemies;

#endif // ENEMY_HPP