#include "Unit.hpp"
#include "../SizeDeducer.hpp"
#include "../PhysicsHelper.hpp" 
#include "../Utils.hpp"
#include "../HealthBar.hpp"
#include "../Weapon.hpp"
#include "../Core.hpp"
#include "../DragonBonesAnimator.hpp"
#include "../Movement.hpp"

#include "../scenes/LevelScene.hpp"

#include <cmath>

Unit::Unit(const std::string& dragonBonesName) :
    m_curses { this },
    m_dragonBonesName { dragonBonesName }
{
    m_weapons.fill(nullptr);
}

Unit::~Unit() {
    for(auto p: m_weapons) {
        if(p) {
            delete p;
            p = nullptr;
        }
    }
}

bool Unit::init() {
    if (!cocos2d::Node::init() ) {
        return false;
    }
    this->scheduleUpdate();

    this->AddAnimator();
    this->AddPhysicsBody();
    const auto body { this->getPhysicsBody() };
    m_movement = std::make_unique<Movement>(body, LevelScene::GRAVITY, LevelScene::JUMP_HEIGHT);
    this->AddWeapons();
    this->setContentSize(m_contentSize);

    /// TODO: move somewhere
    static constexpr float healthBarShift { 5.f };
    const auto bar = HealthBar::create(this);
    const auto shiftY { m_contentSize.height };
    bar->setName("health");
    bar->setPosition(-m_contentSize.width / 2.f, shiftY + healthBarShift);
    this->addChild(bar);

    // add state lable:
    const auto state = cocos2d::Label::createWithTTF("", "fonts/arial.ttf", 15);
    state->setName("state");
    state->setPosition(0.f, bar->getPositionY() + bar->getContentSize().height + 8.f);
    this->addChild(state);    
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

void Unit::ResetForces(bool x, bool y) noexcept {
    if(x && y) m_movement->ResetForce();
    else if(x) m_movement->ResetForceX();
    else if(y) m_movement->ResetForceY();
}

void Unit::MoveAlong(const cocos2d::Vec2& direction) noexcept {
    return this->MoveAlong(direction.x, direction.y);
}

void Unit::MoveAlong(float x, float y) noexcept {
    if (!helper::IsEqual(y, 0.f, 0.0001f)) {
        m_movement->Push(x, y);
    }
    else {
        m_movement->Move(x, y);
    }
}

void Unit::Turn() noexcept {
    m_side = (m_side == Side::LEFT? Side::RIGHT: Side::LEFT);
    m_animator->FlipX();
}

void Unit::LookAt(const cocos2d::Vec2& point) noexcept {
    bool targetIsOnLeft { point.x < this->getPosition().x };
    bool needTurnAround {
        ( targetIsOnLeft && !this->IsLookingLeft() ) ||
        ( !targetIsOnLeft && this->IsLookingLeft() )
    };
    if(needTurnAround) {
        this->Turn();
    }
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
}

void Unit::Attack() {
    if(m_weapons.front()->IsReady() && !this->IsDead()) {
        auto projectilePosition = [this]() -> cocos2d::Rect {
            const auto attackRange { m_weapons.front()->GetRange() };

            auto position = this->getPosition();
            if(m_side == Side::RIGHT) {
                position.x += m_contentSize.width / 2.f;
            }
            else {
                position.x -= m_contentSize.width / 2.f + attackRange;
            }
            // shift a little bit higher to avoid immediate collision with the ground
            position.y += m_contentSize.height * 0.05f;
            cocos2d::Rect attackedArea {
                position,
                cocos2d::Size{ attackRange, m_contentSize.height * 1.05f } // a little bigger than the designed size
            };
            return attackedArea;
        };
        auto pushProjectile = [this](cocos2d::PhysicsBody* body){
            body->setVelocity(this->getPhysicsBody()->getVelocity());
        };
        m_weapons.front()->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

void Unit::UpdateWeapons(const float dt) noexcept {
    if(!this->IsDead()) {
        for(auto& weapon: m_weapons) {
            if(weapon) {
                weapon->UpdateState(dt);
            }
        }
    }
}

void Unit::UpdatePosition(const float dt) noexcept {
    if(!this->IsDead()) {
        m_movement->Update();
    }
}

void Unit::UpdateCurses(const float dt) noexcept {
    m_curses.Update(dt);
}

bool Unit::IsOnGround() const noexcept {
    const auto velocity { this->getPhysicsBody()->getVelocity() };
    constexpr float EPS { 0.000001f };  
    return helper::IsEqual(velocity.y, 0.f, EPS) && m_hasContactWithGround;
}


void Unit::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.1f, 0.1f), 
        {0.f, floorf(m_physicsBodySize.height / 2.f)}
    );
    // body->setMass(25.f);
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
    
    this->addComponent(body);
}

void Unit::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    std::string prefix = m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(chachedArmatureName));
    this->addChild(m_animator);
    m_animator->setScale(0.1f); // TODO: introduce multi-resolution scaling
}

void Unit::AddWeapons() {};
