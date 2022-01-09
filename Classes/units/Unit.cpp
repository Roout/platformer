#include "Unit.hpp"

#include "SizeDeducer.hpp"
#include "PhysicsHelper.hpp" 
#include "Utils.hpp"
#include "HealthBar.hpp"
#include "Weapon.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"
#include "Movement.hpp"

#include "scenes/LevelScene.hpp"

#include <cmath>
#include <cassert>

Unit::Unit(const std::string& dragonBonesName) :
    m_curses { this },
    m_dragonBonesName { dragonBonesName }
{
}

bool Unit::init() {
    if (!cocos2d::Node::init() ) {
        return false;
    }
    // At this moment content size should be initialized
    assert(!cocos2d::Vec2{ m_contentSize }.fuzzyEquals({0.f, 0.f}, 0.01f));

    scheduleUpdate();

    AddAnimator();
    AddPhysicsBody();
    setContentSize(m_contentSize); // must be called after `AddPhysicsBody`
    m_movement = std::make_unique<Movement>(getPhysicsBody()
        , LevelScene::GRAVITY
        , LevelScene::JUMP_HEIGHT);
    AddWeapons();

    /// TODO: move somewhere
    static constexpr float healthBarShift { 5.f };
    const auto bar = HealthBar::create(this);
    const auto shiftY { m_contentSize.height };
    bar->setName("health");
    bar->setPosition(-m_contentSize.width / 2.f, shiftY + healthBarShift);
    addChild(bar);

    // add state lable:
    const auto state = cocos2d::Label::createWithTTF("", "fonts/arial.ttf", 15);
    state->setName("state");
    state->setPosition(0.f, bar->getPositionY() + bar->getContentSize().height + 8.f);
    addChild(state);    
    return true;
}

void Unit::pause() {
    cocos2d::Node::pause();
    m_animator->pause();
}

void Unit::resume() {
    cocos2d::Node::resume();
    m_animator->resume();
}

void Unit::SetMaxSpeed(float speed) noexcept {
    m_movement->SetMaxSpeed(speed);
}

void Unit::MoveAlong(Movement::Direction dir) noexcept {
    assert(m_movement);

    using Move = Movement::Direction;
    switch (dir) {
        case Move::UP: [[fallthrough]];
        case Move::DOWN: {
            m_movement->Push(dir); 
        } break;
        case Move::LEFT: [[fallthrough]];
        case Move::RIGHT: { 
            m_movement->Move(dir);
        } break;
        default: assert(false && "Unreachable");
    }
}

void Unit::Stop(Movement::Axis axis) noexcept {
    assert(m_movement);
    m_movement->Stop(axis);
}

void Unit::Turn() noexcept {
    m_side = (m_side == Side::LEFT? Side::RIGHT: Side::LEFT);
    m_animator->FlipX();
}

void Unit::LookAt(const cocos2d::Vec2& point) noexcept {
    bool targetIsOnLeft { point.x < getPosition().x };
    bool needTurnAround {
        ( targetIsOnLeft && !IsLookingLeft() ) ||
        ( !targetIsOnLeft && IsLookingLeft() )
    };
    if(needTurnAround) {
        Turn();
    }
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
}

void Unit::UpdateWeapons(const float dt) noexcept {
    assert(!IsDead());
    
    for (auto& weapon: m_weapons) {
        if (weapon) {
            weapon->UpdateState(dt);
        }
    }
}

void Unit::UpdatePosition(const float dt) noexcept {
    assert(!IsDead());
    m_movement->Update();
}

void Unit::UpdateCurses(const float dt) noexcept {
    assert(!IsDead());
    m_curses.Update(dt);
}

bool Unit::IsOnGround() const noexcept {
    const auto velocity { getPhysicsBody()->getVelocity() };
    constexpr float EPS { 0.000001f };  
    return helper::IsEqual(velocity.y, 0.f, EPS) && m_hasContactWithGround;
}

void Unit::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.1f, 0.1f), 
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    body->setDynamic(true);
    body->setGravityEnable(true);
    body->setRotationEnable(false);
    
    const cocos2d::Size sensorShapeSize { m_physicsBodySize.width, 8.f };
    const auto sensorShape = cocos2d::PhysicsShapeBox::create(
        sensorShapeSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT
    );
    sensorShape->setSensor(true);
    sensorShape->setTag(Utils::CreateMask(core::CategoryBits::GROUND_SENSOR));
    body->addShape(sensorShape, false);

    const auto hitBoxShape = cocos2d::PhysicsShapeBox::create(
        m_hitBoxSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT,
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    hitBoxShape->setSensor(true);
    hitBoxShape->setTag(Utils::CreateMask(core::CategoryBits::HITBOX_SENSOR));
    body->addShape(hitBoxShape, false);
    
    addComponent(body);
}

void Unit::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    std::string prefix = m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(chachedArmatureName));
    addChild(m_animator);
    m_animator->setScale(0.1f); // TODO: introduce multi-resolution scaling
}

void Unit::AddWeapons() {};
