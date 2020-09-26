#include "Spearman.hpp"

#include "Weapon.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"

#include <memory>

namespace Enemies {

Spearman* Spearman::Spearman::create(size_t id) {
    auto pRet { new (std::nothrow) Spearman(id, core::EntityNames::SPEARMAN) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Spearman::init() {
    if( !Warrior::init() ) {
        return false; 
    }
    m_movement->SetMaxSpeed(150.f);
    return true;
}

Spearman::Spearman(size_t id, const char * name) :
    Warrior{ id, name }
{
    m_designedSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 70.f, 135.f };
}

void Spearman::AddWeapon() {
    const auto damage { 25.f };
    const auto range { 100.f };
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };
    const auto reloadTime { 0.8f };
    m_weapon = std::make_unique<Spear>(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

void Spearman::Attack() {
    if(m_weapon->IsReady() && !this->IsDead()) {
        const auto attackRange { m_weapon->GetRange() };
        const cocos2d::Size spearSize { attackRange, attackRange / 4.f };

        auto position = this->getPosition();
        if(m_side == Side::RIGHT) {
            position.x += this->getContentSize().width / 2.f;
        }
        else {
            position.x -= this->getContentSize().width / 2.f + spearSize.width;
        }
        position.y += this->getContentSize().height / 2.f - spearSize.height / 2.f;

        const cocos2d::Rect attackedArea { position, spearSize };
        m_weapon->LaunchAttack(attackedArea, this->getPhysicsBody()->getVelocity());
    }
}

}// namespace Enemies