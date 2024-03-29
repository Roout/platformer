#include "Stalactite.hpp"
#include "Player.hpp"
#include "Core.hpp"

#include "components/DragonBonesAnimator.hpp"
#include "components/Weapon.hpp"

#include "configs/JsonUnits.hpp"

#include "cocos2d.h"
#include <string>

namespace Enemies {

Stalactite* Stalactite::create(
    size_t id
    , const cocos2d::Size& contentSize
    , float scale
    , const json_models::UnitsStalactite *model
) {
    auto pRet = new (std::nothrow) Stalactite(id
        , contentSize
        , scale
        , cocos2d::RandomHelper::random_int(1, 4)
        , model
    );
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Stalactite::init() {
    if (!Bot::init()) {
        return false;
    }
    getChildByName("health")->removeFromParent();
    getChildByName("state")->removeFromParent();

    m_health = m_model->health;
    return true;
}

void Stalactite::update(float dt) {
    // update components
    cocos2d::Node::update(dt);
    if (!IsDead()) {
        UpdateWeapons(dt);
        TryAttack();
        UpdateCurses(dt);
    }
    UpdateState(dt);
    UpdateAnimation(); 
}

void Stalactite::OnEnemyIntrusion() {
    m_detectEnemy = true;
}

void Stalactite::OnEnemyLeave() {
    m_detectEnemy = false;
}

Stalactite::Stalactite(
    size_t id
    , const cocos2d::Size& contentSize
    , float scale
    , size_t index
    , const json_models::UnitsStalactite *model
) 
    : Bot { id, std::string(core::EntityNames::STALACTITE) + "_" +  std::to_string(index) }
    , m_scale { scale }
    , m_index { index }
    , m_model { model }
{
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width, contentSize.height };
    m_hitBoxSize = m_physicsBodySize;
}

void Stalactite::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::RANGE]->IsReady());
    
    m_weapons[WeaponClass::RANGE]->LaunchAttack();
    m_alreadyAttacked = true;
}

void Stalactite::TryAttack() {
    assert(!IsDead());
    
    if (NeedAttack()) { // attack if possible
        Attack();
    } 
}

bool Stalactite::NeedAttack() const noexcept {
    assert(!IsDead());
    return !m_alreadyAttacked 
        && m_detectEnemy 
        && m_weapons[WeaponClass::RANGE]->IsReady();
}

void Stalactite::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if (m_health <= 0) {
        m_currentState = State::DEAD;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsPreparing()) {
        m_currentState = State::PREPARE_ATTACK;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsAttacking()) {
        m_currentState = State::ATTACK;
    }
    else if (m_currentState != State::ATTACK) {
        m_currentState = State::IDLE;
    }
}

void Stalactite::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        auto isLooped { m_currentState == State::IDLE };
        auto repeatTimes { isLooped ? dragonBones::Animator::INFINITY_LOOP : 1 };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if (IsDead()) {
            OnDeath();
        }
        else if (m_currentState == State::ATTACK) {
            // remove physics body
            removeComponent(getPhysicsBody());
        }
    }
}

void Stalactite::OnDeath() {
    // Just remove physics body. 
    // The base of stalactite will still be visible!
    removeComponent(getPhysicsBody());
    // getChildByName("health")->removeFromParent();
    m_animator->EndWith([this]() {
        runAction(cocos2d::RemoveSelf::create(true));
    });
}

void Stalactite::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.0f, 0.0f), 
        { 0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    body->setDynamic(false);
    body->setGravityEnable(false);

    const auto hitBoxShape = cocos2d::PhysicsShapeBox::create(
        m_hitBoxSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT,
        { 0.f, floorf(m_physicsBodySize.height / 2.f) }
    );
    hitBoxShape->setSensor(true);
    hitBoxShape->setCollisionBitmask(0);
    hitBoxShape->setCategoryBitmask(Utils::EnumCast(core::CategoryBits::HITBOX_SENSOR));
    hitBoxShape->setContactTestBitmask(
        Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE, core::CategoryBits::HITBOX_SENSOR)
    );

    // change masks for physics body
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
    body->setContactTestBitmask(0);
    body->setCollisionBitmask(Utils::CreateMask(core::CategoryBits::BOUNDARY));
    body->addShape(hitBoxShape, false);
    
    addComponent(body);
}

void Stalactite::AddAnimator() {
    // stalactites/stalactite_1/stalactite_1/stalactite_1_tex"
    std::string name = m_dragonBonesName;
    std::string prefix = "stalactites/" + m_dragonBonesName + "/" + m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(name));
    addChild(m_animator);
    m_animator->setScale(m_scale); 
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::IDLE), GetStateName(State::IDLE)),
        std::make_pair(Utils::EnumCast(State::PREPARE_ATTACK), "shaking"),
        std::make_pair(Utils::EnumCast(State::ATTACK), GetStateName(State::ATTACK)), 
        std::make_pair(Utils::EnumCast(State::DEAD), GetStateName(State::DEAD))
    });
}

void Stalactite::AddWeapons() {
    const auto range { m_contentSize.height };
    // TODO: Here a strange mess of durations needed to be fixed
    // The projectile need to be created only when the attack-animation ends
    const auto preparationTime { m_animator->GetDuration(Utils::EnumCast(State::PREPARE_ATTACK)) };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };

    auto genPos = [this]()->cocos2d::Rect {
        auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
        cocos2d::Size stalactite { m_contentSize / m_scale };
        auto position = getPosition();
        // shift y-axis to avoid collision with the ceiling
        return { cocos2d::Vec2(position.x, position.y - stalactite.height * 0.05f) , stalactite};
    };
    auto genVel = [this](cocos2d::PhysicsBody* body) {
        const auto& velocity = m_model->weapons.stalactite.velocity;
        body->setVelocity({ velocity[0], -velocity[1] });
    };

    const auto& stalactite = m_model->weapons.stalactite;
    auto& weapon = m_weapons[WeaponClass::RANGE];
    weapon.reset(new StalactitePart(stalactite.damage
        , range
        , preparationTime
        , attackDuration
        , stalactite.cooldown
        , m_index));
    weapon->AddPositionGenerator(std::move(genPos));
    weapon->AddVelocityGenerator(std::move(genVel));
}


}