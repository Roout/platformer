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
    m_movement { this },
    m_dragonBonesName { dragonBonesName }
{   
}

bool Unit::init() {
    if( !cocos2d::Node::init() ) {
        return false;
    }
    this->scheduleUpdate();

    this->AddAnimator();
    this->AddPhysicsBody(m_designedSize);
    this->setContentSize(m_designedSize);

    /// TODO: move somewhere
    static constexpr float healthBarShift { 15.f };
    const auto bar = HealthBar::create(this);
    bar->setPosition(-this->getContentSize().width / 2.f, this->getContentSize().height + healthBarShift);
    this->addChild(bar);

    // add state lable:
    const auto state = cocos2d::Label::createWithTTF("state", "fonts/arial.ttf", 25);
    state->setName("state");
    state->setPosition(0.f, this->getContentSize().height + 60.f);
    this->addChild(state);
    
    return true;
}

std::string Unit::CreateAnimationName(Act act) {
    std::string animationName { "walk" };
    switch (act) {
        case Act::idle: animationName = "idle"; break;
        case Act::jump: animationName = "jump"; break;
        case Act::move: animationName = "walk"; break;
        case Act::attack: animationName = "attack"; break; 
        case Act::dead: animationName = "dead"; break; 
        default: break;
    }
    return animationName;
}

void Unit::AddPhysicsBody(const cocos2d::Size& size) {
    const auto body = cocos2d::PhysicsBody::createBox(
        size,
        cocos2d::PhysicsMaterial(1.f, 0.f, 0.f), 
        {0.f, size.height / 2.f}
    );
    body->setMass(25.f);
    body->setDynamic(true);
    body->setGravityEnable(true);
    body->setRotationEnable(false);
    
    const cocos2d::Size sensorShapeSize { size.width * 0.9f, 8.f };
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

void Unit::Stop() noexcept {
    m_movement.StopXAxisMove();
}

void Unit::MoveLeft() noexcept {
    m_movement.MoveLeft();
}

void Unit::MoveRight() noexcept {
    m_movement.MoveRight();
}

void Unit::Jump() noexcept {
    m_movement.Jump();
}

void Unit::Turn() noexcept {
    m_currentState.m_side = (m_currentState.m_side == Side::left? Side::right: Side::left);
    this->FlipX();
}

void Unit::FlipX() {
    m_animator->FlipX();
}

void Unit::UpdateDebugLabel() {
    const auto stateLabel = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    if( !stateLabel ) throw "can't find label!";
    stateLabel->setString(CreateAnimationName(m_currentState.m_act));
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
    cocos2d::log(" >>> unit recieve %d damage. Current health is %d.", damage, m_health);
}

void Unit::Attack() {
    if(m_weapon->IsReady() && !this->IsDead()) {
        const auto attackRange { m_weapon->GetRange() };
        
        auto position = this->getPosition();
        if(m_currentState.m_side == Side::right) {
            position.x += this->getContentSize().width / 2.f;
        }
        else {
            position.x -= this->getContentSize().width / 2.f + attackRange;
        }

        static int x { 0 };
        cocos2d::log(" >>> unit attack with sword: %d", ++x );

        const cocos2d::Rect attackedArea {
            position,
            cocos2d::Size{ attackRange, this->getContentSize().height }
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
        m_movement.Update(dt);
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
