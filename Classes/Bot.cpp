#include "Bot.hpp"
#include "PhysicsHelper.hpp"
#include "SizeDeducer.hpp"
#include "Player.hpp"
#include "Weapon.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"

#include <memory>
#include <limits>
#include <algorithm>
#include <unordered_map>

namespace Enemies {

std::string  GetStateName(State state) {
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
    if( !Unit::init()) {
        return false; 
    }
    return true;
}

Bot::Bot(size_t id, const char* dragonBonesName): 
    Unit{ dragonBonesName },
    m_id { id }
{
    m_designedSize = cocos2d::Size{ 80.f, 135.f };
}

bool Bot::NeedAttack() const noexcept {
    bool attackIsReady {
        !this->IsDead() && 
        m_detectEnemy && 
        m_weapon->IsReady()
    };
    auto enemyIsClose = [this]() { 
        const auto target = dynamic_cast<Unit*>(this->getParent()->getChildByName(core::EntityNames::PLAYER));
        // use some simple algorithm to determine whether ф player is close enough to the target
        // to perform an attack
        if( target ) {
            const auto radius = m_weapon->GetRange();
            const cocos2d::Rect lhs { 
                target->getPosition() - cocos2d::Vec2{ target->getContentSize().width / 2.f, 0.f },
                target->getContentSize()
            };
            const cocos2d::Rect rhs { // check attack in both directions
                this->getPosition() - cocos2d::Vec2 { this->getContentSize().width / 2.f + radius, 0.f },
                this->getContentSize() + cocos2d::Size { 2.f * radius, 0.f }
            };
            return lhs.intersectsRect(rhs);
        }
        return false;
    };
    return attackIsReady && enemyIsClose();
}

void Bot::TryAttack() {
    if( this->NeedAttack() ) { // attack if possible
        auto lookAtEnemy { false };
        const auto target = this->getParent()->getChildByName(core::EntityNames::PLAYER);
        if( target->getPosition().x < this->getPosition().x && this->IsLookingLeft() ) {
            lookAtEnemy = true;
        } 
        else if( target->getPosition().x > this->getPosition().x && !this->IsLookingLeft() ) {
            lookAtEnemy = true;
        }
        if( !lookAtEnemy ) {
            this->Turn();
        }
        this->Stop();
        this->Attack();
    } 
}

void Bot::UpdateDebugLabel() noexcept {
    auto state = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    state->setString(GetStateName(m_currentState));
}

void Bot::AttachInfluenceArea(const cocos2d::Rect& area) {
    // Attach influence
    auto influence = Influence::create(this, area);
    influence->setName("Influence");
    this->addComponent(influence);
}

} // namespace Enemies