#include "Influence.hpp"
#include "Enemy.hpp"
#include "Unit.hpp"
#include "Player.hpp"

void WarriorInfluence::OnEnter() {
    const auto target = m_warrior->getParent()->getChildByName(Player::NAME);
    const auto unit = dynamic_cast<Unit*>(target);
    m_warrior->Pursue(unit);
    m_detected = true;
}

void WarriorInfluence::OnExit() {
    m_warrior->Patrol();
    m_detected = false;
}

void WarriorInfluence::Update() {
    if( m_warrior ) {
        const auto target = m_warrior->getParent()->getChildByName(Player::NAME);
        if( target ) { // exist, is alive and kicking
            const auto height { target->getContentSize().height / 2.f };
            const auto point { 
                target->getPosition() + cocos2d::Vec2{ 0.f, height } 
            };
            const auto isInside { m_zone.containsPoint(point) };
            if( !m_detected && isInside) {
                this->OnEnter();
            } else if( m_detected && !isInside) {
                this->OnExit();
            }
        }
        else {
            m_detected = false;
        }
    } 
}
