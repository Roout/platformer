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
#include "Movement.hpp"

#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <cmath>

Player* Player::create(const cocos2d::Size& contentSize) {
    auto pRet { new (std::nothrow) Player(contentSize) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Player::Player(const cocos2d::Size& contentSize) 
    : Unit { "mc" }
{
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size { contentSize.width / 2.f, contentSize.height };
    m_hitBoxSize = contentSize;
}

bool Player::init() {
    if (!Unit::init()) {
        return false; 
    }    
    m_follower = std::make_unique<SmoothFollower>(this);
    m_inputHandler = std::make_unique<UserInputHandler>(this);
    m_movement->SetMaxSpeed(200.f);

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

std::string Player::GetStateName(Player::State state) {
    static std::unordered_map<Player::State, std::string> mapped {
        { Player::State::MELEE_ATTACK, "attack_1" },
        { Player::State::RANGE_ATTACK, "attack_2" },
        { Player::State::IDLE, "idle" },
        { Player::State::JUMP, "jump" },
        { Player::State::WALK, "walk" },
        { Player::State::DEAD, "dead" },
        { Player::State::PREPARE_RANGE_ATTACK, "prep_attack_2" },
        { Player::State::SPECIAL_PHASE_1, "special_phase_1" },
        { Player::State::SPECIAL_PHASE_2, "special_phase_2" },
        { Player::State::SPECIAL_PHASE_3, "special_phase_3" }
    };
    auto it = mapped.find(state);
    return (it != mapped.cend()? it->second: "");        
}

void Player::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    std::string prefix = m_dragonBonesName + "/" + m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(chachedArmatureName));
    m_animator->setScale(0.1f); // TODO: introduce multi-resolution scaling
    m_animator->InitializeAnimations(std::initializer_list<std::pair<size_t, std::string>> {
        { Utils::EnumCast(State::MELEE_ATTACK), GetStateName(State::MELEE_ATTACK) },
        { Utils::EnumCast(State::RANGE_ATTACK), GetStateName(State::RANGE_ATTACK) }, 
        { Utils::EnumCast(State::PREPARE_RANGE_ATTACK), std::string("idle") },
        { Utils::EnumCast(State::DEAD), GetStateName(State::DEAD) },
        { Utils::EnumCast(State::IDLE), GetStateName(State::IDLE) },
        { Utils::EnumCast(State::JUMP), GetStateName(State::JUMP) },
        { Utils::EnumCast(State::WALK), GetStateName(State::WALK) },
        { Utils::EnumCast(State::SPECIAL_PHASE_1), GetStateName(State::SPECIAL_PHASE_1) },
        { Utils::EnumCast(State::SPECIAL_PHASE_2), GetStateName(State::SPECIAL_PHASE_2) },
        { Utils::EnumCast(State::SPECIAL_PHASE_3), GetStateName(State::SPECIAL_PHASE_3) }
    });
    this->addChild(m_animator);
}

void Player::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    
    const auto body { this->getPhysicsBody() };
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PLAYER));
    body->setContactTestBitmask(Utils::CreateMask(core::CategoryBits::PLATFORM));
    body->setCollisionBitmask(
        Utils::CreateMask(core::CategoryBits::BOUNDARY, core::CategoryBits::PLATFORM)
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
        Utils::CreateMask(core::CategoryBits::BOUNDARY, core::CategoryBits::PLATFORM)
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
    if (!helper::IsEqual(y, 0.f, 0.0001f) ) {
        // need to be called earlier because forces will be reseted 
        // and method @IsOnGround will fail
        const auto body { this->getPhysicsBody() };
        if(!helper::IsEqual(body->getVelocity().y, 0.f, 0.001f) ) {
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
    if(m_currentState != m_previousState) {
        int repeatTimes { dragonBones::Animator::INFINITY_LOOP };
        if (m_currentState == State::MELEE_ATTACK 
            || m_currentState == State::JUMP 
            || m_currentState == State::DEAD 
            || m_currentState == State::RANGE_ATTACK 
            || m_currentState == State::PREPARE_RANGE_ATTACK 
            || m_currentState == State::SPECIAL_PHASE_1 
            || m_currentState == State::SPECIAL_PHASE_3 
        ) {
            repeatTimes = 1;
        }
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(this->IsDead()) {
            this->OnDeath();
        }
    }
}

void Player::OnDeath() {
    // remove physics body
    this->removeComponent(this->getPhysicsBody());
    this->getChildByName("health")->removeFromParent();
    m_animator->EndWith([this](){
        // create a death screen
        cocos2d::EventCustom event(DeathScreen::EVENT_NAME);
        this->getEventDispatcher()->dispatchEvent(&event);
        // remove player from screen
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
};

bool Player::IsInvincible() const noexcept {
    return m_isInvincible;
}

void Player::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    const auto velocity { this->IsDead()? cocos2d::Vec2{ 0.f, 0.f } : this->getPhysicsBody()->getVelocity() };
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

    if(m_finishSpecialAttack) {
        this->OnSpecialAttackEnd();
        assert(!m_finishSpecialAttack && "Isn't consumed");
    }
    
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
    else if(m_weapons[WeaponClass::RANGE]->IsAttacking()) {
        m_currentState = State::RANGE_ATTACK;
    }
    else if(m_weapons[WeaponClass::MELEE]->IsPreparing()) {
        m_currentState = State::MELEE_ATTACK;
    }
    else if(m_weapons[WeaponClass::MELEE]->IsAttacking()) {
        m_currentState = State::MELEE_ATTACK;
    }
    else if(m_weapons[WeaponClass::SPECIAL]->IsPreparing()) {
        m_currentState = State::SPECIAL_PHASE_3;
    }
    else if(m_weapons[WeaponClass::SPECIAL]->IsAttacking()) {
        m_currentState = State::SPECIAL_PHASE_3;
    }
    else if(m_scheduleSpecialAttack) {
        m_currentState = State::SPECIAL_PHASE_1;
        // consume scheduled attack
        m_scheduleSpecialAttack = false;
    }
    else if(m_currentState == State::SPECIAL_PHASE_1) {
        // switch to State::SPECIAL_PHASE_2
        // if animation is completed 
        // TODO: try checking whether an animation is ongoing or not
        if(!m_animator->IsPlaying()) {
            m_currentState = State::SPECIAL_PHASE_2;
        }
    }
    else if(m_currentState == State::SPECIAL_PHASE_2) {
        // do nothing as player is charging a special attack
        // it can be aborted if 
        // - [ ] player moved
        // - [ ] release button 
    }
    else if(!this->IsOnGround()) {
        m_currentState = State::JUMP;
    } 
    else if(helper::IsEqual(velocity.x, 0.f, EPS)) {
        m_currentState = State::IDLE;
    } 
    else {
        m_currentState = State::WALK;
    }
}

void Player::AddWeapons() {
    // create weapon (TODO: it should be read from config)
    {
        const auto damage { 25.f };
        const auto range { 80.f };
        const auto animDuration = m_animator->GetDuration(Utils::EnumCast(State::MELEE_ATTACK));
        const auto attackDuration { 0.5f * animDuration };
        const auto preparationTime { animDuration - attackDuration };
        const auto reloadTime { 0.f };
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
        const auto range { 110.f };
        const auto preparationTime { 0.f };
        const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::RANGE_ATTACK))  };
        const auto reloadTime { 0.f };
        m_weapons[WeaponClass::RANGE] = new PlayerFireball(
            damage, 
            range, 
            preparationTime,
            attackDuration,
            reloadTime 
        );
    }
    {
        const auto damage { 55.f };
        const auto range { 180.f };
        const auto animDuration = m_animator->GetDuration(Utils::EnumCast(State::SPECIAL_PHASE_3));
        const auto attackDuration { 0.75f * animDuration };
        const auto preparationTime { animDuration - attackDuration };
        const auto reloadTime { 0.f };
        m_weapons[WeaponClass::SPECIAL] = new PlayerSpecial(
            damage, 
            range, 
            preparationTime,
            attackDuration,
            reloadTime 
        );
    }
};

void Player::RangeAttack() {
    bool usingMelee {
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
            body->setVelocity({ this->IsLookingLeft()? -450.f: 450.f, 0.f });
        };
        m_weapons[WeaponClass::RANGE]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

void Player::MeleeAttack() {
    bool usingRange {
        m_weapons[WeaponClass::RANGE]->IsAttacking() || 
        m_weapons[WeaponClass::RANGE]->IsPreparing()
    };
    if (!usingRange) {
        this->Attack();
    }
}

void Player::SpecialAttack() {
    if (m_weapons[WeaponClass::SPECIAL]->IsReady() && !this->IsDead()) {
        auto projectilePosition = [this]() -> cocos2d::Rect {
            const auto attackRange { m_weapons[WeaponClass::SPECIAL]->GetRange() };
            const cocos2d::Size slashSize { attackRange * 1.8f, attackRange };

            auto position = this->getPosition();
            if (this->IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f ;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += floorf(m_contentSize.height * 0.1f);

            return { position, slashSize };
        };
        
        auto pushProjectile = [this](cocos2d::PhysicsBody* body) {
            body->setVelocity({ this->IsLookingLeft()? -550.f: 550.f, 0.f });
        };
        m_weapons[WeaponClass::SPECIAL]->LaunchAttack(
            std::move(projectilePosition), 
            std::move(pushProjectile)
        );
    }
}

void Player::StartSpecialAttack() {
    bool usingMelee {
        m_weapons[WeaponClass::MELEE]->IsAttacking() || 
        m_weapons[WeaponClass::MELEE]->IsPreparing()
    };
    bool usingRange {
        m_weapons[WeaponClass::RANGE]->IsAttacking() || 
        m_weapons[WeaponClass::RANGE]->IsPreparing()
    };
    bool usingSpecial {
        m_weapons[WeaponClass::SPECIAL]->IsAttacking() || 
        m_weapons[WeaponClass::SPECIAL]->IsPreparing()
    };
    if(!usingMelee && !usingRange && !usingSpecial && m_currentState != State::DEAD) {
        m_scheduleSpecialAttack = true;
    }
}

void Player::OnSpecialAttackEnd() {
    m_finishSpecialAttack = false;
    switch(m_currentState) {
        case State::SPECIAL_PHASE_1: {
            // cancel attack
            m_currentState = State::IDLE;
        } break;
        case State::SPECIAL_PHASE_2: { // this is phase when player is charging sword attack
            // interrupt charging
            // invoke special wweapon attack!
            this->SpecialAttack();
            // switch to phase 3
            m_currentState = State::SPECIAL_PHASE_3;
        } break;
        case State::SPECIAL_PHASE_3: {
            // can be reached when `E` is pressed 
            // and released during this phase
        } break;
        default: break;
    }
}

void Player::FinishSpecialAttack() {
    if( m_currentState == State::SPECIAL_PHASE_1 || 
        m_currentState == State::SPECIAL_PHASE_2 || 
        m_currentState == State::SPECIAL_PHASE_3
    ) {
        m_finishSpecialAttack = true;
    }
}

void Player::Attack() {
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
            position.y += m_contentSize.height * 0.25f;
            cocos2d::Rect attackedArea {
                position,
                cocos2d::Size{ attackRange, m_contentSize.height * 0.5f }
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