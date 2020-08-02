#include "Enemy.hpp"


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
    return true;
}

void Enemies::Warrior::update(float dt) {

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