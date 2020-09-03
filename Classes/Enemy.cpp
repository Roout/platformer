#include "Enemy.hpp"
#include "PhysicsHelper.hpp"
#include "SizeDeducer.hpp"
#include "Player.hpp"
#include "Core.hpp"
#include "Weapon.hpp"
#include "DragonBonesAnimator.hpp"
#include <memory>
#include <limits>
#include <algorithm>

Enemies::Bot* Enemies::Bot::create(size_t id) {
    auto pRet { new (std::nothrow) Bot(id) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Enemies::Bot::init() {
    if( !Unit::init()) {
        return false; 
    }
    m_movement->SetMaxSpeed(130.f);
    return true;
}

void Enemies::Bot::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    // change masks for physics body
    auto body { this->getPhysicsBody() };
    body->setMass(2000.f);
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

void Enemies::Bot::AddWeapon() {
    const auto damage { 25.f };
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

Enemies::Bot::Bot(size_t id): 
    Unit{ "warrior" },
    m_id { id }
{
    m_designedSize = cocos2d::Size{ 80.f, 135.f };
}

bool Enemies::Bot::NeedAttack() const noexcept {
    bool attackIsReady {
        !this->IsDead() && 
        m_detectEnemy && 
        m_weapon->IsReady()
    };
    auto enemyIsClose = [this]() { 
        const auto target = dynamic_cast<Unit*>(this->getParent()->getChildByName(Player::NAME));
        // use some simple algorithm to determine whether ф player is close enough to the target
        // to perform an attack
        if( target ) {
            const auto radius = m_weapon->GetRange();
            const cocos2d::Rect lhs { 
                target->getPosition() - cocos2d::Vec2{ target->getContentSize().width / 2.f, 0.f },
                target->getContentSize()
            };
            const cocos2d::Rect rhs {
                this->getPosition() - cocos2d::Vec2 { this->getContentSize().width / 2.f + radius, 0.f },
                this->getContentSize() + cocos2d::Size { 2.f * radius, 0.f }
            };
            return lhs.intersectsRect(rhs);
        }
        return false;
    };
    return attackIsReady && enemyIsClose();
}

void Enemies::Bot::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::ATTACK), GetStateName(State::ATTACK)),
        std::make_pair(Utils::EnumCast(State::PURSUIT), GetStateName(State::PURSUIT)),
        std::make_pair(Utils::EnumCast(State::PATROL), GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEATH), GetStateName(State::DEATH))
    });
}

void Enemies::Bot::TryAttack() {
    if( this->NeedAttack() ) { // attack if possible
        auto lookAtEnemy { false };
        const auto target = this->getParent()->getChildByName(Player::NAME);
        if( target->getPosition().x < this->getPosition().x && this->IsLookingLeft() ) {
            lookAtEnemy = true;
        } 
        else if( target->getPosition().x > this->getPosition().x && !this->IsLookingLeft() ) {
            lookAtEnemy = true;
        }
        if( !lookAtEnemy ) {
            this->Turn();
        }
        this->Stop();
        this->Attack();
    } 
}

void Enemies::Bot::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if( m_health <= 0 ) {
        m_currentState = State::DEATH;
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

void Enemies::Bot::UpdatePosition(const float dt) noexcept {
    if(m_currentState != State::ATTACK ) {
        m_navigator->Navigate(dt);  // update direction/target if needed
        m_movement->Update(dt);      // apply forces
    }
}

void Enemies::Bot::UpdateAnimation() {
    if( this->IsDead() ) {
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
        // remove from screen
        this->runAction(cocos2d::RemoveSelf::create());
    } 
    else if(m_currentState != m_previousState) {
        auto repeatTimes { m_currentState == State::ATTACK? 1 : dragonBones::Animator::INFINITY_LOOP };
        m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
    }
}

void Enemies::Bot::update(float dt) {
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

std::string  Enemies::Bot::GetStateName(Bot::State state) {
    static std::unordered_map<Bot::State, std::string> mapped {
        { Bot::State::PATROL, "walk" },
        { Bot::State::ATTACK, "attack" },
        { Bot::State::PURSUIT, "walk" },
        { Bot::State::DEATH, "death" }
    };
    auto it = mapped.find(state);
    return (it != mapped.cend()? it->second: "");        
}

void Enemies::Bot::UpdateDebugLabel() noexcept {
    auto state = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    state->setString(this->GetStateName(m_currentState));
}

void Enemies::Bot::AttachNavigator(
    const cocos2d::Size& mapSize, 
    float tileSize,
    path::Supplement * const supplement
) {
    m_navigator = std::make_unique<Navigator>(mapSize, tileSize);
    m_navigator->Init(this, supplement);
}

void Enemies::Bot::AttachInfluenceArea(const cocos2d::Rect& area) {
    // Attach influence
    auto influence = Influence::create(this, area);
    influence->setName("Influence");
    this->addComponent(influence);
}

void Enemies::Bot::Pursue(Unit * target) noexcept {
    m_navigator->Pursue(target);
}

void Enemies::Bot::Patrol() noexcept {
    m_navigator->Patrol();
}

void Enemies::Bot::OnEnemyIntrusion() {
    m_detectEnemy = true;
    auto target = dynamic_cast<Unit*>(this->getParent()->getChildByName(Player::NAME));
    this->Pursue(target);
}

void Enemies::Bot::OnEnemyLeave() {
    m_detectEnemy = false;
    this->Patrol();
}