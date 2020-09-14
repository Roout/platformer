#include "Influence.hpp"
#include "Bot.hpp"
#include "Player.hpp"
#include "Core.hpp"

Influence* Influence::create(
    Enemies::Bot* bot, 
    const cocos2d::Rect& zone
) {
    auto pRet = new(std::nothrow) Influence(bot, zone);
    if(pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Influence::Influence(
    Enemies::Bot* bot, 
    const cocos2d::Rect& zone
) :
    m_bot{ bot },
    m_zone{ zone }
{
}

void Influence::OnIntrusion() {
    m_bot->OnEnemyIntrusion();
    m_detected = true;
}

void Influence::OnLeave() {
    m_bot->OnEnemyLeave();
    m_detected = false;
}

void Influence::update(float dt) {
    if( m_bot && !m_bot->IsDead() ) {
        const auto target = m_bot->getParent()->getChildByName(core::EntityNames::PLAYER);
        if( target ) { // exist, is alive and kicking
            const auto height { target->getContentSize().height / 2.f };
            const auto point { 
                target->getPosition() + cocos2d::Vec2{ 0.f, height } 
            };
            const auto isInside { m_zone.containsPoint(point) };
            if( !m_detected && isInside) {
                this->OnIntrusion();
            } 
            else if( m_detected && !isInside) {
                this->OnLeave();
            }
        }
        else {
            m_detected = false;
        }
    } 
}
