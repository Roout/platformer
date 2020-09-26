#include "Unit.hpp"
#include "SizeDeducer.hpp"
#include "PhysicsHelper.hpp" 
#include "Utils.hpp"
#include "HealthBar.hpp"
#include "ResourceManagement.hpp"
#include "Weapon.hpp"
#include "Core.hpp"
#include "DragonBonesAnimator.hpp"

Unit::Unit(const std::string& dragonBonesName) :
    m_curses { this },
    m_dragonBonesName { dragonBonesName }
{   
}

bool Unit::init() {
    if( !cocos2d::Node::init() ) {
        return false;
    }
    this->scheduleUpdate();

    this->AddAnimator();
    this->AddPhysicsBody();
    const auto body { this->getPhysicsBody() };
    m_movement = std::make_unique<Movement>(body);
    this->AddWeapon();
    this->setContentSize(m_designedSize);

    /// TODO: move somewhere
    static constexpr float healthBarShift { 5.f };
    const auto bar = HealthBar::create(this);
    const auto shiftY { m_designedSize.height };
    bar->setName("health");
    bar->setPosition(-m_designedSize.width / 2.f, shiftY + healthBarShift);
    this->addChild(bar);

    // add state lable:
    const auto state = cocos2d::Label::createWithTTF("state", "fonts/arial.ttf", 25);
    state->setName("state");
    state->setPosition(0.f, bar->getPositionY() + bar->getContentSize().height + 15.f);
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

void Unit::MoveAlong(const cocos2d::Vec2& direction) noexcept {
    return this->MoveAlong(direction.x, direction.y);
}

void Unit::MoveAlong(float x, float y) noexcept {
    // Horizontal move
    if(x == -1.f) {
        m_movement->MoveLeft();
    }
    else if(x == 1.f) {
        m_movement->MoveRight();
    }
    // Vertical move
    if(y == -1.f) {
        m_movement->MoveDown();
    }
    else if(y == 1.f) {
        m_movement->MoveUp();
    }
    // Stop
    if( x == 0.f && y == 0.f) {
        m_movement->Stop();
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
    if(m_weapon->IsReady() && !this->IsDead()) {
        const auto attackRange { m_weapon->GetRange() };

        auto position = this->getPosition();
        if(m_side == Side::RIGHT) {
            position.x += this->getContentSize().width / 2.f;
        }
        else {
            position.x -= this->getContentSize().width / 2.f + attackRange;
        }
        // shift a little bit higher to avoid immediate collision with the ground
        position.y += this->getContentSize().height * 0.05f;
        const cocos2d::Rect attackedArea {
            position,
            cocos2d::Size{ attackRange, this->getContentSize().height * 0.9f }
        };
        m_weapon->LaunchAttack(attackedArea, this->getPhysicsBody()->getVelocity());
    }
}

void Unit::UpdateWeapon(const float dt) noexcept {
    if(!this->IsDead()) {
        m_weapon->UpdateState(dt);
    }
}

void Unit::UpdatePosition(const float dt) noexcept {
    if(!this->IsDead()) {
        m_movement->Update(dt);
    }
}

void Unit::UpdateCurses(const float dt) noexcept {
    m_curses.Update(dt);
}

bool Unit::IsOnGround() const noexcept {
    const auto velocity { this->getPhysicsBody()->getVelocity() };
    constexpr float EPS { 0.000001f };  
    return helper::IsEquel(velocity.y, 0.f, EPS) && m_hasContactWithGround;
}


void Unit::AddPhysicsBody() {
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.f, 0.2f), 
        {0.f, m_designedSize.height / 2.f}
    );
    body->setMass(25.f);
    body->setDynamic(true);
    body->setGravityEnable(true);
    body->setRotationEnable(false);
    
    const cocos2d::Size sensorShapeSize { m_physicsBodySize.width * 0.9f, 8.f };
    const auto sensorShape = cocos2d::PhysicsShapeBox::create(
        sensorShapeSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT
    );
    sensorShape->setSensor(true);
    sensorShape->setTag(
        Utils::CreateMask(core::CategoryBits::GROUND_SENSOR)
    );
    body->addShape(sensorShape, false);
    this->addComponent(body);
}

void Unit::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(chachedArmatureName));
    this->addChild(m_animator);
    m_animator->setScale(0.2f); // TODO: introduce multi-resolution scaling
}

void Unit::AddWeapon() {};
