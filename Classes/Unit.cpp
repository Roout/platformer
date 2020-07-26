#include "Unit.hpp"
#include "cocos2d.h"
#include "SizeDeducer.hpp"
#include "PhysicsHelper.hpp" 
#include "Utils.hpp"

#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"

Unit* Unit::create(const cocos2d::Size& size) {
    auto pRet = new (std::nothrow) Unit(size);
    if(pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Unit::Unit(const cocos2d::Size& size) :
    m_curses { this },
    m_movement { this }
{   
    // Create body
    this->CreateBody(size);
    
    // Create weapon
    const int damage { 25 };
    const int range { SizeDeducer::GetInstance().GetAdjustedSize(20) };
    const float reloadTime { m_maxAttackTime };
    m_weapon = std::make_unique<Sword>( damage, range, reloadTime );
}

bool Unit::init() {
    if( !cocos2d::Node::init() ) {
        return false;
    }
    this->scheduleUpdate();

    // load animation data and build the armature
    /// TODO: move this to function
    const auto factory = dragonBones::CCFactory::getFactory();
    if(auto bonesData = factory->getDragonBonesData("mc"); bonesData == nullptr) {
        factory->loadDragonBonesData("mc/mc_ske.json");
    }
    if(auto texture = factory->getTextureAtlasData("mc"); texture == nullptr) {
        factory->loadTextureAtlasData("mc/mc_tex.json");
    }
    auto armatureDisplay = factory->buildArmatureDisplay("Armature", "mc");

    // TODO: scale factor depends on device resolution so it can'be predefined constant.
    constexpr auto designedScaleFactor { 0.2f };
    const auto adjustedScaleFactor { 
        SizeDeducer::GetInstance().GetAdjustedSize(designedScaleFactor) 
    };
    armatureDisplay->setName("Armature");
    
    this->addChild(armatureDisplay);

    // adjust animation
    armatureDisplay->setScale( adjustedScaleFactor );
    armatureDisplay->getAnimation()->play("idle");

    // add state lable:
    auto state = cocos2d::Label::createWithTTF("state", "fonts/arial.ttf", 25);
    state->setName("state");
    state->setPosition(0.f, this->getContentSize().height + 60.f);
    this->addChild(state);
    
    return true;
}

std::string Unit::CreateAnimationName(Act act) {
    std::string animationName { "walk" };
    switch (act)
    {
        case Act::idle: animationName = "idle"; break;
        case Act::jump: animationName = "jump"; break;
        case Act::move: animationName = "walk"; break;
        case Act::attack: animationName = "attack"; break; 
        case Act::dead: animationName = "dead"; break; 
        default: break;
    }
    return animationName;
}

void Unit::CreateBody(const cocos2d::Size& size) {
    const auto body = cocos2d::PhysicsBody::createBox(
        size,
        cocos2d::PhysicsMaterial(1.f, 0.f, 0.f), 
        {0.f, size.height / 2.f}
    );
    body->setMass(25.f);
    body->setDynamic(true);
    body->setGravityEnable(true);
    body->setRotationEnable(false);
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::HERO)
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::ENEMY, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PROJECTILE, 
            core::CategoryBits::PLATFORM 
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::PLATFORM,
            core::CategoryBits::TRAP
        )
    );
    
    const cocos2d::Size sensorShapeSize { size.width / 2.f, 10.f };
    const auto sensorShape = cocos2d::PhysicsShapeBox::create(
        sensorShapeSize, 
        cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT
    );
    sensorShape->setSensor(true);
    sensorShape->setCategoryBitmask(
        Utils::CreateMask(
            core::CategoryBits::HERO_SENSOR
        )
    );
    sensorShape->setCollisionBitmask(0);
    sensorShape->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PLATFORM
        )
    );
    body->addShape(sensorShape, false);

    this->addComponent(body);
    this->setContentSize(size);
}

void Unit::FlipX(const Unit::Side currentSide) {
    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("Armature")
    );

    if(const auto isTurnedRight = armatureDisplay->getArmature()->getFlipX(); 
        isTurnedRight && currentSide == Unit::Side::left
    ) {
        armatureDisplay->getArmature()->setFlipX(false);
    } 
    else if( !isTurnedRight && currentSide == Unit::Side::right) { 
        armatureDisplay->getArmature()->setFlipX(true);
    }
}

void Unit::UpdateAnimation() {
    auto armatureDisplay = dynamic_cast<dragonBones::CCArmatureDisplay*>(
        this->getChildByName("Armature")
    );
    
    if( m_previousState.m_side != m_currentState.m_side ) {
        this->FlipX(m_currentState.m_side);
    }

    // run animation
    const std::string animationName { CreateAnimationName(m_currentState.m_act) };
    const bool oneTime { 
        m_currentState.m_act == Unit::Act::jump || 
        m_currentState.m_act == Unit::Act::attack 
    };
    if( m_previousState.m_act != m_currentState.m_act ) {
        armatureDisplay->getAnimation()->play(animationName, oneTime? 1: -1);
    }
}

void Unit::update(float dt) {
    // update order is important
    this->UpdateWeapon(dt);
    this->UpdatePosition(dt);
    this->UpdateAnimation();
    this->UpdateState(dt);
    this->UpdateCurses(dt);

    // Debug >> Update state:
    auto stateLabel = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    if( !stateLabel ) throw "can't find label!";
    stateLabel->setString(CreateAnimationName(m_currentState.m_act));
}

void Unit::RecieveDamage(int damage) noexcept {
    m_health -= damage;
    cocos2d::log(" >>> unit recieve %d damage. Current health is %d.", damage, m_health);
}

void Unit::MeleeAttack() {
    if( m_weapon->CanAttack() ) {
        // update attack direction and position for idle case
        auto position = cocos2d::Vec2::ZERO;
        auto direction = cocos2d::Vec2::ZERO;
        if(m_currentState.m_side == Side::right) {
            direction.x = 1.f;
            position.x += this->getContentSize().width;
        }
        else {
            direction.x = -1.f;
            position.x -= this->getContentSize().width;
        }

        static int x { 0 };
        cocos2d::log(" >>> unit attack with sword: %d", ++x );

        m_weapon->Attack(position, direction);
        // update state
        m_currentState.m_act = Act::attack;
        // update cooldown
        m_attackTime = m_maxAttackTime;
    }
}

void Unit::UpdateWeapon(const float dt) noexcept {
    m_weapon->Update(dt);
}

void Unit::UpdatePosition(const float dt) noexcept {
    m_movement.Update(dt);
}

bool Unit::IsOnGround() const noexcept {
    const auto direction { this->getPhysicsBody()->getVelocity() };
    constexpr float EPS { 0.00001f };
    
    auto isOnGround { helper::IsEquel(direction.y, 0.f, EPS) };
    return isOnGround && m_hasContactWithGround;
}

void Unit::UpdateState(const float dt) noexcept {
    const auto direction { this->getPhysicsBody()->getVelocity() };
    constexpr float EPS { 0.00001f };

    m_previousState = m_currentState;

    // update character direction
    if( helper::IsPositive(direction.x, EPS) ) {
        m_currentState.m_side = Side::right;
    } else if( helper::IsNegative(direction.x, EPS) ) {
        m_currentState.m_side = Side::left;
    }

    // update character state
    if( m_currentState.m_act == Act::attack ) {
        m_attackTime -= dt;
        if( helper::IsPositive(m_attackTime, EPS) ) {
            // exit to not change an attack state
            return; 
        }
    }

    if( !this->IsOnGround() ) {
        m_currentState.m_act = Act::jump;
    } else if( !helper::IsEquel(direction.x, 0.f, EPS) ) {
        m_currentState.m_act = Act::move;
    } else {
        m_currentState.m_act = Act::idle;
    }
}

void Unit::UpdateCurses(const float dt) noexcept {
    m_curses.Update(dt);
    // Check health points:
    if( m_health <= 0) {
        m_previousState.m_act = m_currentState.m_act;
        m_currentState.m_act = Act::dead;
        // Do smth on death
    }
}

Player* Player::create(const cocos2d::Size& size) {
    auto pRet { new (std::nothrow) Player(size) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Player::Player(const cocos2d::Size& sz) :
    Unit { sz }
{}


bool Player::init() {
    if( !Unit::init()) {
        return false; 
    }
    m_follower = std::make_unique<SmoothFollower>(this);
    return true;
}

void Player::setPosition(const cocos2d::Vec2& position) {
    Node::setPosition(position.x, position.y);
    m_follower->Reset();
}

void Player::update(float dt) {
    Unit::update(dt);
    m_follower->UpdateMapPosition(dt);
}
