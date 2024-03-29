#include "Bot.hpp"
#include "Player.hpp"
#include "PhysicsHelper.hpp"
#include "Core.hpp"
#include "Settings.hpp"

#include "components/Influence.hpp"
#include "components/Weapon.hpp"
#include "components/DragonBonesAnimator.hpp"
#include "components/Movement.hpp"

#include <memory>
#include <limits>
#include <algorithm>
#include <unordered_map>

namespace Enemies {

std::string GetStateName(State state) {
    static std::unordered_map<State, std::string> mapped {
        { State::PATROL,            "walk" },
        { State::PURSUIT,           "walk" },
        { State::WALK,              "walk" },
        { State::DASH,              "dash" },
        
        { State::IDLE,              "idle" },
        { State::PREPARE_ATTACK,    "prepare_attack" },
        { State::ATTACK,            "attack" },
        { State::DEAD,              "dead" },

        { State::FIREBALL_ATTACK,   "attack_1" },
        { State::FIRECLOUD_ATTACK,  "attack_2" },
        { State::SWEEP_ATTACK,      "attack_3" },
        { State::BASIC_ATTACK,      "basic_attack" },
        { State::BASIC_WALK,        "basic_walk" },

        { State::INIT,              "walk_1" },
        { State::EARLY,             "walk_2" },
        { State::MID,               "walk_3" },
        { State::LATE,              "walk_4" }
    };
    auto it = mapped.find(state);
    return (it != mapped.cend()? it->second: "undefined");        
}

bool Bot::init() {
    if (!Unit::init()) {
        return false; 
    }
    return true;
}

Bot::Bot(size_t id, const std::string& dragonBonesName): 
    Unit{ dragonBonesName },
    m_id { id }
{
    m_contentSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 70.f, 135.f };
    m_hitBoxSize = m_physicsBodySize;
}

bool Bot::NeedAttack() const noexcept {
    assert(!IsDead());
    constexpr auto MELEE { 0U };
    bool attackIsReady { 
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
    assert(!IsDead());
    const auto target = getParent()->getChildByName(core::EntityNames::PLAYER);
    if (target && NeedAttack()) { // attack if possible
        LookAt(target->getPosition());
        Stop(Movement::Axis::XY);
        Attack();
    } 
}

void Bot::UpdateDebugLabel() noexcept {
    using Debug = settings::DebugMode;
    const auto state = this->getChildByName<cocos2d::Label*>("state");
    bool isEnabled = Debug::GetInstance().IsEnabled(Debug::OptionKind::kState);
    if (state && isEnabled) {
        state->setString(GetStateName(m_currentState));
    }
    else if (state && !isEnabled) {
        state->setString("");
    }
}

void Bot::AttachInfluenceArea(const cocos2d::Rect& area) {
    // Attach influence
    m_influence = Influence::create(this, area);
    m_influence->setName("Influence");
    this->addComponent(m_influence);
}

} // namespace Enemies