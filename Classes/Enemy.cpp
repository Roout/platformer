#include "Enemy.hpp"
#include "PhysicsHelper.hpp"

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
            core::CategoryBits::TRAP,
            core::CategoryBits::PLATFORM,
            core::CategoryBits::PROJECTILE, 
            core::CategoryBits::GROUND_SENSOR
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
            core::CategoryBits::PLATFORM,
            core::CategoryBits::HERO
        )
    );

    return true;
}

void Enemies::Warrior::UpdateState(const float dt) noexcept {
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

    m_currentState.m_act = Act::move;
}

Enemies::Warrior::Warrior(const cocos2d::Size& size, size_t id): 
    Unit{ size, "warrior" },
    m_id { id }
{
    m_movement.SetMaxSpeed(130.f);
}

void Enemies::Warrior::update(float dt) {
    m_navigator->Navigate(dt);    
    Unit::update(dt);
}

void Enemies::Warrior::AttachNavigator(
    const cocos2d::Size& mapSize, 
    float tileSize,
    path::Forest * const forest
) {
    m_navigator = std::make_unique<Navigator>(mapSize, tileSize);
    m_navigator->Init(this, forest);
}