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

    // Create weapon
    const auto damage { 25.f };
    const auto range { 60.f };
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(Act::attack)) };
    const auto reloadTime { 0.5f };
    m_weapon = std::make_unique<Axe>(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
    return true;
}

void Enemies::Bot::AddPhysicsBody(const cocos2d::Size& size) {
    Unit::AddPhysicsBody(size);
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

void Enemies::Bot::pause() {
    cocos2d::Node::pause();
    m_animator->pause();
}

void Enemies::Bot::resume() {
    cocos2d::Node::resume();
    m_animator->resume();
}

void Enemies::Bot::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;
    if( m_health <= 0 ) {
        m_currentState.m_act = Act::dead;
    }
    else if( m_weapon->IsAttacking() ) {
        m_currentState.m_act = Act::attack;
    }
    else {
        m_currentState.m_act = Act::move;
    }
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
    std::string chachedArmatureName = m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(chachedArmatureName));
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Act::attack), "attack"),
        std::make_pair(Utils::EnumCast(Act::dead), "dead"),
        std::make_pair(Utils::EnumCast(Act::move), "walk")
    });
    this->addChild(m_animator);
    m_animator->setScale(0.2f); // TODO: introduce multi-resolution scaling
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

void Enemies::Bot::UpdatePosition(const float dt) noexcept {
    if(m_currentState.m_act != Act::attack ) {
        m_navigator->Navigate(dt);  // update direction/target if needed
        m_movement->Update(dt);      // apply forces
    }
}

void Enemies::Bot::UpdateAnimation() {
    if( m_currentState.m_act != m_previousState.m_act ) {
        if( m_currentState.m_act == Act::dead ) {
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
        else {
            int repeatTimes { dragonBones::Animator::INFINITY_LOOP };
            if( m_currentState.m_act == Act::attack || 
                m_currentState.m_act == Act::dead
            ) {
                repeatTimes = 1;
            }
            m_animator->Play(Utils::EnumCast(m_currentState.m_act), repeatTimes);
        }
    }
}

void Enemies::Bot::update(float dt) {
    cocos2d::Node::update(dt);
    this->UpdateDebugLabel();
    this->UpdateWeapon(dt);
    this->UpdatePosition(dt); 
    this->UpdateCurses(dt);
    this->TryAttack();
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

void Enemies::Bot::AttachNavigator(
    const cocos2d::Size& mapSize, 
    float tileSize,
    path::Supplement * const supplement
) {
    m_navigator = std::make_unique<Navigator>(mapSize, tileSize);
    m_navigator->Init(this, supplement);
}

void Enemies::Bot::AttachInfluenceArea(
    const cocos2d::Size& mapSize, 
    float tileSize,
    path::Supplement * const supplement
) {
    const auto mapHeight { static_cast<int> (mapSize.height) };
    const auto warriorTilePosition { this->getPosition() / tileSize };
    
    auto CoordsFromTile = [&mapHeight](int x, int y) {
        return cocos2d::Vec2 {
            static_cast<float>(x),
            static_cast<float>(mapHeight - y - 1)
        };
    };
    // Find closest area to this unit
    size_t rectDataIndex { 0 };
    size_t closest { std::numeric_limits<size_t>::max() };
    float minDistance { std::numeric_limits<float>::max() };  
    for(const auto& rectData: supplement->areas) {
        // Extract 2 tiles coordinates in Tiled program system: bot left & top right
        const auto coords { CoordsFromTile (rectData[0], rectData[1]) };
        const auto distance = fabs(warriorTilePosition.x - coords.x) + fabs(warriorTilePosition.y - coords.y);
        if( minDistance > distance ) {
            minDistance = distance;
            closest = rectDataIndex;
        }
        rectDataIndex++;
    }
    // Create from them a rectangular
    if( closest != std::numeric_limits<size_t>::max() ) {
        const auto& rectData = supplement->areas[closest];
        const auto pointA { CoordsFromTile (rectData[0], rectData[1]) };
        const auto pointB { CoordsFromTile (rectData[2], rectData[3]) };
        const cocos2d::Rect rect {
            { pointA.x * tileSize, pointA.y * tileSize },
            { (pointB.x - pointA.x + 1) * tileSize, (pointB.y - pointA.y + 1) * tileSize }
        };
        // Attach influence
        auto component = Influence::create(this, rect);
        component->setName("Influence");
        this->addComponent(component);
    }
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