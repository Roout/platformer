#include "Bot.hpp"
#include "Influence.hpp"
#include "PhysicsHelper.hpp"
#include "SizeDeducer.hpp"
#include "Player.hpp"
#include "Weapon.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"
#include "Movement.hpp"

#include <memory>
#include <limits>
#include <algorithm>
#include <unordered_map>

namespace Enemies {

std::string GetStateName(State state) {
    static std::unordered_map<State, std::string> mapped {
        { State::PATROL,            "walk" },
        { State::PURSUIT,           "walk" },
        { State::IDLE,              "idle" },
        { State::PREPARE_ATTACK,    "prepare_attack" },
        { State::ATTACK,            "attack" },
        { State::DEAD,              "dead" }
    };
    auto it = mapped.find(state);
    return (it != mapped.cend()? it->second: "");        
}

bool Bot::init() {
    if (!Unit::init()) {
        return false; 
    }
    return true;
}

Bot::Bot(size_t id, const char* dragonBonesName): 
    Unit{ dragonBonesName },
    m_id { id }
{
    m_contentSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 70.f, 135.f };
    m_hitBoxSize = m_physicsBodySize;
}

bool Bot::NeedAttack() const noexcept {
    constexpr auto MELEE { 0U };
    bool attackIsReady {
        !this->IsDead() && 
        m_detectEnemy && 
        m_weapons[MELEE]->IsReady()
    };
    auto enemyIsClose = [this, MELEE]() { 
        const auto target = this->getParent()->getChildByName<const Unit*>(core::EntityNames::PLAYER);
        // use some simple algorithm to determine whether a player is close enough to the target
        // to perform an attack
        if(target && !target->IsDead()) {
            const auto radius = m_weapons[MELEE]->GetRange();
            const cocos2d::Rect lhs { 
                target->getPosition() - cocos2d::Vec2{ target->GetHitBox().width / 2.f, 0.f },
                target->GetHitBox()
            };
            const cocos2d::Rect rhs { // check attack in both directions
                this->getPosition() - cocos2d::Vec2 { m_contentSize.width / 2.f + radius, 0.f },
                m_contentSize + cocos2d::Size { 2.f * radius, 0.f }
            };
            return lhs.intersectsRect(rhs);
        }
        return false;
    };
    return attackIsReady && enemyIsClose();
}

void Bot::TryAttack() {
    const auto target = this->getParent()->getChildByName(core::EntityNames::PLAYER);
    if( target && this->NeedAttack() ) { // attack if possible
        this->LookAt(target->getPosition());
        this->MoveAlong(0.f, 0.f);
        this->Attack();
    } 
}

void Bot::UpdateDebugLabel() noexcept {
    auto state = this->getChildByName<cocos2d::Label*>("state");
    state->setString(GetStateName(m_currentState));
}

void Bot::AttachInfluenceArea(const cocos2d::Rect& area) {
    // Attach influence
    m_influence = Influence::create(this, area);
    m_influence->setName("Influence");
    this->addComponent(m_influence);
}

} // namespace Enemies