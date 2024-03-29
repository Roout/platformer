#include "Player.hpp"

#include "SmoothFollower.hpp"
#include "UserInputHandler.hpp"
#include "PhysicsHelper.hpp"
#include "Utils.hpp"
#include "Core.hpp"
#include "Settings.hpp"

#include "components/Weapon.hpp"
#include "components/DragonBonesAnimator.hpp"
#include "components/Movement.hpp"
#include "components/Dash.hpp"

#include "scenes/DeathScreen.hpp"

#include "configs/JsonUnits.hpp"

#include <unordered_map>
#include <algorithm>
#include <cassert>
#include <cmath>

Player* Player::create(const cocos2d::Size& contentSize
    , const json_models::Player *model) 
{
    auto pRet { new (std::nothrow) Player(contentSize, model) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Player::Player(const cocos2d::Size& contentSize
    , const json_models::Player *model
) 
    : Unit { core::EntityNames::PLAYER }
    , m_model { model }
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
    m_movement->SetMaxSpeed(m_model->maxSpeed);
    
    m_health = m_model->health;
    // add dash component
    m_dash = Dash::create(m_model->weapons.dash.cooldown
        , m_model->maxSpeed
        , m_model->weapons.dash.velocity[0]);
    addComponent(m_dash);
    // =====
    return true;
}

void Player::RecieveDamage(int damage) noexcept {
    using Debug = settings::DebugMode;
    if (!Debug::GetInstance().IsEnabled(Debug::OptionKind::kInvicible)) {
        Unit::RecieveDamage(damage);
    }
}

std::string Player::GetStateName(Player::State state) {
    static std::unordered_map<Player::State, std::string> mapped {
        { Player::State::MELEE_ATTACK_1, "attack_1_1" },
        { Player::State::MELEE_ATTACK_2, "attack_1_2" },
        { Player::State::MELEE_ATTACK_3, "attack_1_3" },
        { Player::State::RANGE_ATTACK, "attack_2" },
        { Player::State::IDLE, "idle" },
        { Player::State::JUMP, "jump" },
        { Player::State::WALK, "walk" },
        { Player::State::DEAD, "dead" },
        { Player::State::DASH, "dash" },
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
        { Utils::EnumCast(State::MELEE_ATTACK_1), GetStateName(State::MELEE_ATTACK_1) },
        { Utils::EnumCast(State::MELEE_ATTACK_2), GetStateName(State::MELEE_ATTACK_2) },
        { Utils::EnumCast(State::MELEE_ATTACK_3), GetStateName(State::MELEE_ATTACK_3) },
        { Utils::EnumCast(State::RANGE_ATTACK), GetStateName(State::RANGE_ATTACK) }, 
        { Utils::EnumCast(State::PREPARE_RANGE_ATTACK), std::string("idle") },
        { Utils::EnumCast(State::DEAD), GetStateName(State::DEAD) },
        { Utils::EnumCast(State::IDLE), GetStateName(State::IDLE) },
        { Utils::EnumCast(State::JUMP), GetStateName(State::JUMP) },
        { Utils::EnumCast(State::DASH), GetStateName(State::DASH) },
        { Utils::EnumCast(State::WALK), GetStateName(State::WALK) },
        { Utils::EnumCast(State::SPECIAL_PHASE_1), GetStateName(State::SPECIAL_PHASE_1) },
        { Utils::EnumCast(State::SPECIAL_PHASE_2), GetStateName(State::SPECIAL_PHASE_2) },
        { Utils::EnumCast(State::SPECIAL_PHASE_3), GetStateName(State::SPECIAL_PHASE_3) }
    });
    addChild(m_animator);
}

void Player::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    
    const auto body { getPhysicsBody() };
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
    if (!IsDead()) {
        m_movement->Update();
        m_follower->UpdateMapPosition(dt);
    }
}

void Player::MoveAlong(Movement::Direction dir) noexcept {
    using Move = Movement::Direction;

    if (dir == Move::UP || dir == Move::DOWN) {
        // need to be called earlier because forces will be reseted 
        // and method @IsOnGround will fail
        const auto body { getPhysicsBody() };
        bool moveAlongY = !helper::IsEqual(body->getVelocity().y, 0.f, 0.001f);
        if (moveAlongY) {
            m_hasContactWithGround = false;
        }
        m_movement->Stop(Movement::Axis::XY);
        m_movement->Push(dir);
        // reset all active weapons
        std::for_each(m_weapons.begin(), m_weapons.end(), [](std::unique_ptr<Weapon>& weapon) {
            if (weapon && (weapon->IsPreparing() || weapon->IsAttacking())) {
                weapon->ForceReload();
            }
        });
        FinishSpecialAttack();
        // reset states to invoke JUMP animation (if the player already jumping!)
        m_previousState = State::IDLE;
        m_currentState = State::IDLE;
    }
    else {
        m_movement->Move(dir);
    }
}

void Player::pause() {
    Unit::pause();
    if (!IsDead()) {
        Stop(Movement::Axis::XY);
        // prevent to being called onExit() when the player is dead and is being detached!
        m_inputHandler->Reset();
    }
}

void Player::update(float dt) {
    cocos2d::Node::update(dt);
     
    UpdateDebugLabel();
    if (!IsDead()) {
        UpdateWeapons(dt);
        UpdatePosition(dt); 
        UpdateCurses(dt);
    }
    UpdateState(dt);
    UpdateAnimation(); 
}

void Player::UpdateDebugLabel() noexcept {
    using Debug = settings::DebugMode;
    const auto state = getChildByName<cocos2d::Label*>("state");
    bool isEnabled = Debug::GetInstance().IsEnabled(Debug::OptionKind::kState);
    if (state && isEnabled) {
        state->setString(GetStateName(m_currentState));
    }
    else if (state && !isEnabled) {
        state->setString("");
    }
}

void Player::UpdateAnimation() {
    const std::array<State, 10u> oneTimers {
        State::MELEE_ATTACK_1,
        State::MELEE_ATTACK_2,
        State::MELEE_ATTACK_3,
        State::JUMP,
        State::DASH,
        State::DEAD,
        State::RANGE_ATTACK,
        State::PREPARE_RANGE_ATTACK,
        State::SPECIAL_PHASE_1,
        State::SPECIAL_PHASE_3
    };
    if (m_currentState != m_previousState) {
        int repeatTimes { dragonBones::Animator::INFINITY_LOOP };
        if (std::find(oneTimers.cbegin(), oneTimers.cend(), m_currentState) != oneTimers.cend()) {
            repeatTimes = 1;
        }
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if (IsDead()) {
            OnDeath();
        }
    }
}

void Player::OnDeath() {
    // remove physics body
    removeComponent(getPhysicsBody());
    getChildByName("health")->removeFromParent();
    m_animator->EndWith([this]() {
        // create a death screen
        cocos2d::EventCustom event(DeathScreen::EVENT_NAME);
        getEventDispatcher()->dispatchEvent(&event);
        // remove player from screen
        runAction(cocos2d::RemoveSelf::create(true));
    });
};

void Player::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    static const std::array<State, 3> basicSwordAttacks { 
        State::MELEE_ATTACK_1, 
        State::MELEE_ATTACK_2, 
        State::MELEE_ATTACK_3
    };
    const auto velocity { IsDead()? cocos2d::Vec2{ 0.f, 0.f } : getPhysicsBody()->getVelocity() };
    static constexpr float EPS { 0.001f };

    // check whether we're out of level bounds
    const cocos2d::Rect boundary {
        getParent()->getPosition() - m_contentSize,
        getParent()->getContentSize() + m_contentSize * 2.f 
    };
    const cocos2d::Rect player {
        getParent()->getPosition() + getPosition(),
        m_contentSize
    };

    if (m_finishSpecialAttack) {
        OnSpecialAttackEnd();
        assert(!m_finishSpecialAttack && "Isn't consumed");
    }

    if (!player.intersectsRect(boundary)) { 
        // out of level boundaries
        m_currentState = State::DEAD;
        m_health = 0;
    }
    else if (m_health <= 0) {
        m_currentState = State::DEAD;
    }
    else if (m_dash->IsRunning()) {
        m_currentState = State::DASH;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsPreparing()) {
        m_currentState = State::PREPARE_RANGE_ATTACK;
    }
    else if (m_weapons[WeaponClass::RANGE]->IsAttacking()) {
        m_currentState = State::RANGE_ATTACK;
    }
    else if (m_weapons[WeaponClass::MELEE]->IsPreparing()) {
        if (std::find(basicSwordAttacks.cbegin(), basicSwordAttacks.cend(), m_currentState) == basicSwordAttacks.cend()) {
            m_currentState = basicSwordAttacks[cocos2d::RandomHelper::random_int(0, 2)];
        }
    }
    else if (m_weapons[WeaponClass::MELEE]->IsAttacking()) {
        // TODO: no need to update anything
        // m_currentState = State::MELEE_ATTACK;
    }
    else if (m_weapons[WeaponClass::SPECIAL]->IsPreparing()) {
        m_currentState = State::SPECIAL_PHASE_3;
    }
    else if (m_weapons[WeaponClass::SPECIAL]->IsAttacking()) {
        m_currentState = State::SPECIAL_PHASE_3;
    }
    else if (m_scheduleSpecialAttack) {
        m_currentState = State::SPECIAL_PHASE_1;
        // consume scheduled attack
        m_scheduleSpecialAttack = false;
    }
    else if (m_currentState == State::SPECIAL_PHASE_1) {
        // switch to State::SPECIAL_PHASE_2
        // if animation is completed 
        // TODO: try checking whether an animation is ongoing or not
        if (!m_animator->IsPlaying()) {
            m_currentState = State::SPECIAL_PHASE_2;
        }
    }
    else if (m_currentState == State::SPECIAL_PHASE_2) {
        // do nothing as player is charging a special attack
        // it can be aborted if 
        // - [x] player moved
        // - [x] release button 
    }
    else if (!IsOnGround()) {
        m_currentState = State::JUMP;
    } 
    else if (helper::IsEqual(velocity.x, 0.f, EPS)) {
        m_currentState = State::IDLE;
    } 
    else {
        m_currentState = State::WALK;
    }
}

void Player::AddWeapons() {
    // create weapon (TODO: it should be read from config)
    {
        const float EPS { 0.001f };      
        // the duration of each type of simple attack MUST BE same
        const float animDuration[3] = {
            m_animator->GetDuration(Utils::EnumCast(State::MELEE_ATTACK_1)),
            m_animator->GetDuration(Utils::EnumCast(State::MELEE_ATTACK_2)),
            m_animator->GetDuration(Utils::EnumCast(State::MELEE_ATTACK_3))
        };

        assert(helper::IsEqual(animDuration[0], animDuration[1], EPS) 
            && helper::IsEqual(animDuration[1], animDuration[2], EPS) 
            && "Sword attack animations have different duration"
        );
        
        const auto attackDuration { 0.5f * animDuration[0] };
        const auto preparationTime { animDuration[0] - attackDuration };

        auto genPos = [this]() -> cocos2d::Rect {
            auto attackRange { m_weapons.front()->GetRange() };

            auto position = getPosition();
            if (m_side == Side::RIGHT) {
                position.x += m_contentSize.width / 2.f;
            }
            else {
                position.x -= m_contentSize.width / 2.f + attackRange;
            }
            position.y += m_contentSize.height * 0.25f;
            cocos2d::Rect attackedArea { position,
                cocos2d::Size{ attackRange, m_contentSize.height * 0.5f }
            };
            return attackedArea;
        };
        auto genVel = [this](cocos2d::PhysicsBody* body) {
            body->setVelocity(getPhysicsBody()->getVelocity());
        };

        const auto& sword = m_model->weapons.sword;
        auto& weapon = m_weapons[WeaponClass::MELEE];
        weapon.reset(new Sword(sword.damage
            , sword.range
            , preparationTime
            , attackDuration
            , sword.cooldown));
        weapon->AddPositionGenerator(std::move(genPos));
        weapon->AddVelocityGenerator(std::move(genVel));
    }
    {
        const auto preparationTime { 0.f };
        const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::RANGE_ATTACK))  };

        auto genPos = [this]() -> cocos2d::Rect {
            auto attackRange { m_weapons[WeaponClass::RANGE]->GetRange() };
            cocos2d::Size fireballSize { attackRange, floorf(attackRange * 0.8f) };

            auto position = getPosition();
            if (IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f ;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += floorf(m_contentSize.height * 0.3f);

            return { position, fireballSize };
        };
        auto genVel = [this](cocos2d::PhysicsBody* body) {
            const auto& velocity = m_model->weapons.spell.projectile.velocity;
            body->setVelocity({ IsLookingLeft()? -velocity[0]: velocity[0], velocity[1] });
        };

        const auto& spell = m_model->weapons.spell;
        auto& weapon = m_weapons[WeaponClass::RANGE];
        weapon.reset(new PlayerFireball(spell.projectile.damage
            , spell.range
            , preparationTime
            , attackDuration
            , spell.cooldown));
        weapon->AddPositionGenerator(std::move(genPos));
        weapon->AddVelocityGenerator(std::move(genVel));
    }
    {
        const auto animDuration = m_animator->GetDuration(Utils::EnumCast(State::SPECIAL_PHASE_3));
        const auto attackDuration { 0.75f * animDuration };
        const auto preparationTime { animDuration - attackDuration };

        auto genPos = [this]() -> cocos2d::Rect {
            auto attackRange { m_weapons[WeaponClass::SPECIAL]->GetRange() };
            cocos2d::Size slashSize { attackRange * 1.8f, attackRange };

            auto position = getPosition();
            if (IsLookingLeft()) {
                position.x -= m_contentSize.width / 2.f ;
            }
            else {
                position.x += m_contentSize.width / 2.f;
            }
            position.y += floorf(m_contentSize.height * 0.1f);

            return { position, slashSize };
        };
        auto genVel = [this](cocos2d::PhysicsBody* body) {
            const auto& velocity = m_model->weapons.special.projectile.velocity; 
            body->setVelocity({ IsLookingLeft()? -velocity[0]: velocity[0], velocity[1] });
        };

        const auto& special = m_model->weapons.special;
        auto& weapon = m_weapons[WeaponClass::SPECIAL];
        weapon.reset(new PlayerSpecial(special.projectile.damage
            , special.range
            , preparationTime
            , attackDuration
            , special.cooldown));
        weapon->AddPositionGenerator(std::move(genPos));
        weapon->AddVelocityGenerator(std::move(genVel));
    }
};

void Player::InitiateDash() {
    float duration { m_animator->GetDuration(Utils::EnumCast(State::DASH)) };
    bool canDash = !m_dash->IsOnCooldown() 
        && std::none_of(m_weapons.cbegin(), m_weapons.cend(), [](const std::unique_ptr<Weapon>& weapon) {
            return weapon && (weapon->IsPreparing() || weapon->IsAttacking());
    });

    if (canDash) {
        m_dash->Initiate(duration);
    }
}

void Player::RangeAttack() {
    bool usingMelee {
        m_weapons[WeaponClass::MELEE]->IsAttacking() || 
        m_weapons[WeaponClass::MELEE]->IsPreparing()
    };
    bool canAttack {
        !usingMelee 
        && m_weapons[WeaponClass::RANGE]->IsReady() 
        && !IsDead()
        && m_currentState != State::DASH
    };
    if (canAttack) {
        m_weapons[WeaponClass::RANGE]->LaunchAttack();
    }
}

void Player::MeleeAttack() {
    assert(m_weapons.front());

    bool usingRange {
        m_weapons[WeaponClass::RANGE]->IsAttacking() || 
        m_weapons[WeaponClass::RANGE]->IsPreparing()
    };
    bool canAttack {
        !usingRange 
        && !IsDead()
        && m_weapons.front()->IsReady()
        && m_currentState != State::DASH
    };
    if (canAttack) {
        Attack();
    }
}

void Player::SpecialAttack() {
    bool canAttack {
        m_weapons[WeaponClass::SPECIAL]->IsReady()
        && !IsDead()
        && m_currentState != State::DASH
    };
    if (canAttack) {
        m_weapons[WeaponClass::SPECIAL]->LaunchAttack();
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

    if (!usingMelee 
        && !usingRange 
        && !usingSpecial 
        && m_currentState != State::DEAD 
        && m_currentState != State::WALK
        && m_currentState != State::DASH
    ) {
        m_scheduleSpecialAttack = true;
    }
}

void Player::OnSpecialAttackEnd() {
    m_finishSpecialAttack = false;
    m_scheduleSpecialAttack = false; 

    switch (m_currentState) {
        case State::SPECIAL_PHASE_1: {
            // cancel attack
            m_currentState = State::IDLE;
        } break;
        case State::SPECIAL_PHASE_2: { // this is phase when player is charging sword attack
            // interrupt charging
            // invoke special weapon attack!
            SpecialAttack();
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
    if (m_currentState == State::SPECIAL_PHASE_1 || 
        m_currentState == State::SPECIAL_PHASE_2 || 
        m_currentState == State::SPECIAL_PHASE_3
    ) {
        m_finishSpecialAttack = true;
    }
}

void Player::Attack() {
    assert(!IsDead());
    assert(m_weapons[WeaponClass::MELEE]->IsReady());

    m_weapons[WeaponClass::MELEE]->LaunchAttack();
}