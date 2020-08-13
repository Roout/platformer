#include "Enemy.hpp"
#include "PhysicsHelper.hpp"
#include "SizeDeducer.hpp"
#include "Player.hpp"
#include "Core.hpp"
#include "Weapon.hpp"
#include <memory>
#include <limits>

Enemies::Warrior* Enemies::Warrior::create(const cocos2d::Size& size, size_t id) {
    auto pRet { new (std::nothrow) Warrior(size, id) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Enemies::Warrior::init() {
    if( !Unit::init()) {
        return false; 
    }

    // change masks for physics body
    const auto body { this->getPhysicsBody() };
    body->setMass(2000.f);
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::ENEMY)
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
            // core::CategoryBits::HERO, 
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PLATFORM 
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
    //        core::CategoryBits::GROUND_SENSOR,
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
        //    core::CategoryBits::HERO,
            core::CategoryBits::BOUNDARY,
            core::CategoryBits::PLATFORM
        )
    );

    return true;
}

void Enemies::Warrior::UpdateState(const float dt) noexcept {
    // const auto direction { this->getPhysicsBody()->getVelocity() };
    constexpr float EPS { 0.00001f };

    m_previousState = m_currentState;

    // // update character direction
    // if( helper::IsPositive(direction.x, EPS) ) {
    //     m_currentState.m_side = Side::right;
    // } else if( helper::IsNegative(direction.x, EPS) ) {
    //     m_currentState.m_side = Side::left;
    // }

    // update character state
    if( m_currentState.m_act == Act::attack ) {
        m_attackTime -= dt;
        if( helper::IsPositive(m_attackTime, EPS) ) {
            // exit to not change an attack state
            return; 
        }
    }

    m_currentState.m_act = Act::move;
}

Enemies::Warrior::Warrior(const cocos2d::Size& size, size_t id): 
    Unit{ size, "warrior" },
    m_id { id }
{
    m_movement.SetMaxSpeed(130.f);

    // Create weapon
    m_maxAttackTime = 0.6f;
    const int damage { 5 };
    const int range { SizeDeducer::GetInstance().GetAdjustedSize(90) };
    const float reloadTime { 0.7f };
    m_weapon = std::make_unique<Axe>( damage, range, reloadTime );
}

bool Enemies::Warrior::NeedAttack() noexcept {
    const auto attackIsReady = m_influence.EnemyDetected() && m_weapon->CanAttack();
    const auto target = dynamic_cast<Unit*>(this->getParent()->getChildByName(Player::NAME));
    const auto enemyIsClose = [this, target]() { 
        // use some simple algorithm to determine whether player is close enough to
        // perform an attack
        if( target ) {
            const auto radius = static_cast<float>(m_weapon->GetRange());
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

void Enemies::Warrior::update(float dt) {
    this->UpdateCurses(dt);
    this->UpdateWeapon(dt);
    m_influence.Update(); // detect player
    if( this->NeedAttack() ) { // attack if possible
        auto lookAtEnemy { false };
        const auto target = dynamic_cast<Unit*>(this->getParent()->getChildByName(Player::NAME));
        if( target->getPosition().x < this->getPosition().x && this->IsLookingLeft() ) {
            lookAtEnemy = true;
        } else if( target->getPosition().x > this->getPosition().x && !this->IsLookingLeft() ) {
            lookAtEnemy = true;
        }
        if( !lookAtEnemy ) {
            this->Turn();
        }
        this->Attack();
        m_movement.StopXAxisMove();
    } 
    if(m_currentState.m_act != Act::attack ) {
        m_navigator->Navigate(dt); // move if needed
        this->UpdatePosition(dt); 
    }
    this->UpdateAnimation(); 
    this->UpdateState(dt);

    // Debug >> Update state:
    const auto stateLabel = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    if( !stateLabel ) throw "can't find label!";
    stateLabel->setString(CreateAnimationName(m_currentState.m_act));
}

void Enemies::Warrior::AttachNavigator(
    const cocos2d::Size& mapSize, 
    float tileSize,
    path::Supplement * const supplement
) {
    m_navigator = std::make_unique<Navigator>(mapSize, tileSize);
    m_navigator->Init(this, supplement);
}

void Enemies::Warrior::AttachInfluenceArea(
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
        m_influence.Attach(this, rect);
    }
}

void Enemies::Warrior::Pursue(Unit * target) noexcept {
    m_navigator->Pursue(target);
}

void Enemies::Warrior::Patrol() noexcept {
    m_navigator->Patrol();
}