#include "Warrior.hpp"

#include "Player.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"
#include "Weapon.hpp"
#include "Influence.hpp"
#include "UnitMovement.hpp"

#include "cocos2d.h"

namespace Enemies {

Warrior* Warrior::create(size_t id) {
    auto pRet { new (std::nothrow) Warrior(id, core::EntityNames::WARRIOR) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Warrior::Warrior(size_t id, const char* dragonBonesName) :
    Bot{ id, dragonBonesName }
{
    m_designedSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 70.f, 135.f };
}


bool Warrior::init() {
    if( !Bot::init() ) {
        return false; 
    }
    m_movement->SetMaxSpeed(130.f);
    return true;
}

void Warrior::update(float dt) {
     // update components
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdateWeapon(dt);
    this->UpdatePosition(dt); 
    this->UpdateCurses(dt);
    this->TryAttack();
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

/// Unique to warrior
void Warrior::AttachNavigator(Path&& path) {
    // process a default paths waypoints to avoid jumping:
    // align them with own Y-axis position
    for(auto& point: path.m_waypoints) {
        point.y = this->getPosition().y;
    }
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    this->Patrol();
}

void Warrior::Pursue(cocos2d::Node * target) noexcept {
    if(!this->IsDead() && target) {
        const auto shift { 
            target->getContentSize().width / 2.f    // shift to bottom left\right corner
            + m_weapon->GetRange() * 0.8f           // shift by weapon length (not 1.0f to be able to reach the target by attack!)
            + this->getContentSize().width / 2.f    
        };
        // possible destinations (bottom middle of this unit)
        const float xTargets[2] = { 
            target->getPosition().x + shift,
            target->getPosition().x - shift,
        };
        // choose the closest target in out influence field!
        const float xDistances[2] = {
            fabs(xTargets[0] - this->getPosition().x),
            fabs(xTargets[1] - this->getPosition().x)
        };
        const float xShift { this->getContentSize().width * 0.4f };
        // -xShift for left-bottom corner of this unit 
        // +xShift for right-bottom corner of this unit 
        // So if influence contains either of these corners than the unit won't fall down for sure!
        const bool acceptable[2] = {
            m_influence->Contains({ xTargets[0] - xShift, this->getPosition().y }) || 
            m_influence->Contains({ xTargets[0] + xShift, this->getPosition().y }),

            m_influence->Contains({ xTargets[1] - xShift, this->getPosition().y }) || 
            m_influence->Contains({ xTargets[1] + xShift, this->getPosition().y })
        };
        // find the closest point from the ones where unit won't fall down
        int choosenIndex { -1 };
        float xDistance { xDistances[0] };
        for(int i = 0; i < 2; i++) {
            if(acceptable[i] &&  xDistances[i] <= xDistance) {
                choosenIndex = i;
                xDistance = xDistances[i];
            }
        }
        auto destination = this->getPosition();
        if(choosenIndex != -1) { // every target lead to falling down or other shit
            destination.x = xTargets[choosenIndex];
        }
        // move to target along X-axis;
        // Pass own Y-axis coordinate to not move along Y-axis
        m_navigator->VisitCustomPoint(destination);
    }
}

void Warrior::Patrol() noexcept {
    m_navigator->FollowPath();
}

/// Bot interface
void Warrior::OnEnemyIntrusion() {
    m_detectEnemy = true;
    auto target = this->getParent()->getChildByName(core::EntityNames::PLAYER);
    this->Pursue(target);
}

void Warrior::OnEnemyLeave() {
    m_detectEnemy = false;
    this->Patrol();
}

void Warrior::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if( m_health <= 0 ) {
        m_currentState = State::DEAD;
    }
    else if( m_weapon->IsAttacking() ) {
        m_currentState = State::ATTACK;
    }
    else if( m_detectEnemy ) {
        m_currentState = State::PURSUIT;
    }
    else {
        m_currentState = State::PATROL;
    }
}

void Warrior::UpdatePosition(const float dt) noexcept {
    auto initiateAttack { m_weapon->IsAttacking() || m_weapon->IsPreparing() };
    if(!this->IsDead() && !initiateAttack) {
        if(m_detectEnemy) { // update target
            auto target = this->getParent()->getChildByName(core::EntityNames::PLAYER);
            this->Pursue(target);
        }
        // update
        m_navigator->Update(dt);
        m_movement->Update(dt);
    }
}

void Warrior::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        const auto isOneTimeAttack { 
            m_currentState == State::ATTACK || 
            m_currentState == State::DEAD
        };
        const auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(this->IsDead()) {
            this->OnDeath();
        }
    }
}

void Warrior::OnDeath() {
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
}

void Warrior::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    auto body { this->getPhysicsBody() };
    body->setMass(25.f);
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::ENEMY)
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PLATFORM 
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::TRAP,
            core::CategoryBits::PLATFORM,
            core::CategoryBits::PROJECTILE
        )
    );
    const auto sensor { 
        body->getShape(Utils::EnumCast(
            core::CategoryBits::GROUND_SENSOR)
        ) 
    };
    sensor->setCollisionBitmask(0);
    sensor->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::GROUND_SENSOR)
    );
    sensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PLATFORM
        )
    );
}

void Warrior::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::ATTACK),  GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::PURSUIT), GetStateName(State::PURSUIT)),
        std::make_pair(Utils::EnumCast(State::PATROL),  GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEAD),   GetStateName(State::DEAD))
    });
}

void Warrior::AddWeapon() {
    const auto damage { 2.f };
    const auto range { 60.f };
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };
    const auto reloadTime { 0.5f };
    m_weapon = std::make_unique<Axe>(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
}

} // namespace Enemies