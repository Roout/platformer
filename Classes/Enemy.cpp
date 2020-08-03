#include "Enemy.hpp"
#include "PhysicsHelper.hpp"

Enemies::Warrior* Enemies::Warrior::create(const cocos2d::Size& size) {
    auto pRet { new (std::nothrow) Warrior(size) };
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
            core::CategoryBits::HERO, 
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

Enemies::Warrior::Warrior(const cocos2d::Size& size): 
    Unit{size, "warrior"},
    m_id { m_generator.Next() }
{
}


// void Enemies::Warrior::CreateBody(const cocos2d::Size& size) {
//     const auto body = cocos2d::PhysicsBody::createBox(
//         size,
//         cocos2d::PhysicsMaterial(1.f, 0.f, 0.f), 
//         {0.f, size.height / 2.f}
//     );
//     body->setMass(25.f);
//     body->setDynamic(true);
//     body->setGravityEnable(true);
//     body->setRotationEnable(false);
//     body->setCategoryBitmask(
//         Utils::CreateMask(core::CategoryBits::ENEMY)
//     );
//     body->setCollisionBitmask(
//         Utils::CreateMask(
//             core::CategoryBits::HERO, 
//             core::CategoryBits::ENEMY, 
//             core::CategoryBits::BOUNDARY, 
//             core::CategoryBits::PROJECTILE, 
//             core::CategoryBits::PLATFORM 
//         )
//     );
//     body->setContactTestBitmask(
//         Utils::CreateMask(
//             core::CategoryBits::PLATFORM,
//             core::CategoryBits::TRAP
//         )
//     );
    
//     const cocos2d::Size sensorShapeSize { size.width / 2.f, 10.f };
//     const auto sensorShape = cocos2d::PhysicsShapeBox::create(
//         sensorShapeSize, 
//         cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT
//     );
//     sensorShape->setSensor(true);
//     sensorShape->setCategoryBitmask(
//         Utils::CreateMask(
//             core::CategoryBits::GROUND_SENSOR
//         )
//     );
//     sensorShape->setCollisionBitmask(0);
//     sensorShape->setContactTestBitmask(
//         Utils::CreateMask(
//             core::CategoryBits::BOUNDARY,
//             core::CategoryBits::PLATFORM
//         )
//     ); 
//     body->addShape(sensorShape, false);

//     this->addComponent(body);
//     this->setContentSize(size);
// }