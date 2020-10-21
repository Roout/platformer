#include "Player.hpp"
#include "SmoothFollower.hpp"
#include "UserInputHandler.hpp"
#include "PhysicsHelper.hpp"
#include "Utils.hpp"
#include "Core.hpp"
#include "SizeDeducer.hpp"
#include "Weapon.hpp"
#include "DeathScreen.hpp"
#include "DragonBonesAnimator.hpp"
#include "UnitMovement.hpp"

#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <cmath>

Player* Player::create() {
    auto pRet { new (std::nothrow) Player() };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Player::Player() :
    Unit { "mc" }
{
    m_contentSize = cocos2d::Size{ 80.f, 135.f };
    m_physicsBodySize = cocos2d::Size{ 40.f, 135.f };
    m_hitBoxSize = m_contentSize;
}

bool Player::init() {
    if (!Unit::init()) {
        return false; 
    }    
    m_follower = std::make_unique<SmoothFollower>(this);
    m_inputHandler = std::make_unique<UserInputHandler>(this);
    m_movement->SetMaxSpeed(400.f);

    // handle INVINCIBLE event
    auto listener = cocos2d::EventListenerCustom::create("INVINCIBLE", [this](cocos2d::EventCustom *) {
        this->m_isInvincible = m_isInvincible? false : true;
    });
    const auto dispatcher = this->getEventDispatcher();
    dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    return true;
}

void Player::RecieveDamage(int damage) noexcept {
    if(!m_isInvincible) {
        Unit::RecieveDamage(damage);
    }
}

std::string  Player::GetStateName(Player::State state) {
    static std::unordered_map<Player::State, std::string> mapped {
        { Player::State::MELEE_ATTACK, "attack_1" },
        { Player::State::RANGE_ATTACK, "attack_2" },
        { Player::State::PREPARE_RANGE_ATTACK, "prep_attack_2" },
        { Player::State::IDLE, "idle" },
        { Player::State::JUMP, "jump" },
        { Player::State::WALK, "walk" },
        { Player::State::DEAD, "dead" }
    };
    auto it = mapped.find(state);
    return (it != mapped.cend()? it->second: "");        
}

void Player::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::MELEE_ATTACK), "attack_1"),
        std::make_pair(Utils::EnumCast(State::RANGE_ATTACK), "attack_2"),       // some problems with animator...sorry
        std::make_pair(Utils::EnumCast(State::PREPARE_RANGE_ATTACK), "idle"),   // same here....
        std::make_pair(Utils::EnumCast(State::DEAD), "dead"),
        std::make_pair(Utils::EnumCast(State::IDLE), "idle"),
        std::make_pair(Utils::EnumCast(State::JUMP), "jump"),
        std::make_pair(Utils::EnumCast(State::WALK), "walk")
    });
}

void Player::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    
    const auto body { this->getPhysicsBody() };
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLAYER));
    body->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY 
            , core::CategoryBits::PLATFORM 
        )
    );

    const auto hitBoxTag { Utils::CreateMask(core::CategoryBits::HITBOX_SENSOR) };
    const auto hitBoxSensor { body->getShape(hitBoxTag) };
    hitBoxSensor->setCollisionBitmask(0);
    hitBoxSensor->setCategoryBitmask(hitBoxTag);
    hitBoxSensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::ENEMY_PROJECTILE
            , core::CategoryBits::TRAP
            , core::CategoryBits::HITBOX_SENSOR
        )
    );

    const auto groundSensorTag { Utils::EnumCast(core::CategoryBits::GROUND_SENSOR) };
    const auto groundSensor { body->getShape(groundSensorTag) };
    groundSensor->setCollisionBitmask(0);
    groundSensor->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::GROUND_SENSOR));
    groundSensor->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLATFORM
        )
    );
}

void Player::setPosition(const cocos2d::Vec2& position) {
    Node::setPosition(position.x, position.y);
    m_follower->Reset();
}

void Player::UpdatePosition(const float dt) noexcept {
    if(!this->IsDead()) {
        m_movement->Update(dt);
        m_follower->UpdateMapPosition(dt);
    }
}

void Player::MoveAlong(float x, float y) noexcept {
    if (!helper::IsEquel(y, 0.f, 0.0001f) ) {
        // need to be called earlier because forces will be reseted 
        // and method @IsOnGround will fail
        const auto body { this->getPhysicsBody() };
        if(!helper::IsEquel(body->getVelocity().y, 0.f, 0.001f) ) {
            m_hasContactWithGround = false;
        }
        m_movement->ResetForceY();
        m_movement->Push(x, y);
    }
    else {
        m_movement->Move(x, y);
    }
}

void Player::pause() {
    Unit::pause();
    if(!this->IsDead()) {
        // prevent to being called onExit() when the player is dead and is being detached!
        m_inputHandler->Reset();
    }
}

void Player::update(float dt) {
    cocos2d::Node::update(dt);
     
    this->UpdateDebugLabel();
    this->UpdateWeapons(dt);
    this->UpdatePosition(dt); 
    this->UpdateCurses(dt);
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

void Player::UpdateDebugLabel() noexcept {
    const auto state = this->getChildByName<cocos2d::Label*>("state");
    state->setString(this->GetStateName(m_currentState));
}

void Player::UpdateAnimation() {
    if(this->IsDead()) {
        this->OnDeath();
    } 
    else if(m_currentState != m_previousState) {
        int repeatTimes { dragonBones::Animator::INFINITY_LOOP };
        if (m_currentState == State::MELEE_ATTACK 
            || m_currentState == State::JUMP 
            || m_currentState == State::RANGE_ATTACK 
            || m_currentState == State::PREPARE_RANGE_ATTACK 
        ) {
            repeatTimes = 1;
        }
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
    }
}

void Player::OnDeath() {
    // emit particles
    const auto emitter = cocos2d::ParticleSystemQuad::create("particle_texture.plist");
    emitter->setAutoRemoveOnFinish(true);
    /// TODO: adjust for the multiresolution
    emitter->setScale(0.4f);
    emitter->setPositionType(cocos2d::ParticleSystem::PositionType::RELATIVE);
    emitter->setPosition(this->getPosition());
    this->getParent()->addChild(emitter, 9);

    // remove physics body
    this->removeComponent(this->getPhysicsBody());
    // create a death screen
    cocos2d::EventCustom event(DeathScreen::EVENT_NAME);
    this->getEventDispatcher()->dispatchEvent(&event);
    // remove player from screen
    this->runAction(cocos2d::RemoveSelf::create());
};

bool Player::IsInvincible() const noexcept {
    return m_isInvincible;
}

void Player::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    const auto velocity { this->getPhysicsBody()->getVelocity() };
    static constexpr float EPS { 0.001f };

    // check whether we're out of level bounds
    const cocos2d::Rect boundary {
        this->getParent()->getPosition() - m_contentSize,
        this->getParent()->getContentSize() + m_contentSize * 2.f 
    };
    const cocos2d::Rect player {
        this->getParent()->getPosition() + this->getPosition(),
        m_contentSize
    };
    
    if(!player.intersectsRect(boundary)) { 
        // out of level boundaries
        m_currentState = State::DEAD;
        m_health = 0;
    }
    else if(m_health <= 0) {
        m_currentState = State::DEAD;
    }
    else if(m_weapons[WeaponClass::RANGE]->IsPreparing()) {
        m_currentState = State::PREPARE_RANGE_ATTACK;
    }
    else if(m_weapons[WeaponClass::MELEE]->IsAttacking()) {
        m_currentState = State::MELEE_ATTACK;
    }
    else if(m_weapons[WeaponClass::RANGE]->IsAttacking()) {
        m_currentState = State::RANGE_ATTACK;
    }
    else if(!this->IsOnGround()) {
        m_currentState = State::JUMP;
    } 
    else if(helper::IsEquel(velocity.x, 0.f, EPS)) {
        m_currentState = State::IDLE;
    } 
    else {
        m_currentState = State::WALK;
    }
}

void Player::AddWeapons() {
    // create weapon (it should be read from config)
    {
        const auto damage { 25.f };
        const auto range { 70.f };
        const auto preparationTime { 0.f };
        const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::MELEE_ATTACK)) };
        const auto reloadTime { 0.1f };
        m_weapons[WeaponClass::MELEE] = new Sword(
            damage, 
            range, 
            preparationTime,
            attackDuration,
            reloadTime 
        );
    }
    {
        const auto damage { 25.f };
        const auto range { 130.f };
        const auto preparationTime { 0.f };
        const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::RANGE_ATTACK))  };
        const auto reloadTime { 2.f };
        m_weapons[WeaponClass::RANGE] = new Fireball(
            damage, 
            range, 
            preparationTime,
            attackDuration,
            reloadTime 
        );
    }
};

void Player::RangeAttack() {
    bool usingMelee = {
        m_weapons[WeaponClass::MELEE]->IsAttacking() || 
        m_weapons[WeaponClass::MELEE]->IsPreparing()
    };
    if (!usingMelee && m_weapons[WeaponClass::RANGE]->IsReady() && !this->IsDead()) {
        auto projectilePosition = [this]() -> cocos2d::Rect {
            const auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
            const cocos2d::Size fireballSize { attackRange, floorf(attackRange * 0.8f) };

            auto position = this->getPosition();
            if (this->IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f ;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += floorf(m_contentSize.height * 0.3f);

            return { position, fireballSize };
        };
        
        auto pushProjectile = [this](cocos2d::PhysicsBody* body) {
            body->setVelocity({ this->IsLookingLeft()? -750.f: 750.f, 0.f });
        };
        m_weapons[WeaponClass::RANGE]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

void Player::MeleeAttack() {
    bool usingRange = {
        m_weapons[WeaponClass::RANGE]->IsAttacking() || 
        m_weapons[WeaponClass::RANGE]->IsPreparing()
    };
    if (!usingRange) {
        Unit::Attack();
    }
}