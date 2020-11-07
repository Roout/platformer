#include "Weapon.hpp"
#include "Projectile.hpp"
#include "Utils.hpp"
#include "Player.hpp"
#include "Core.hpp"

/**
 * TODO: 
 * - remove run-time dependency on UI. It shouldn't be attached to the scene here
 * - remove code repetition: everything except mask is the same!
 */

void Sword::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");

    const auto proj = Projectile::create(this->GetDamage());
    const auto projectile = m_extractor();
    const auto body = proj->AddPhysicsBody(projectile.size);
    proj->setPosition(projectile.origin);
    m_modifier(body);
    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::PROPS
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::ENEMY_PROJECTILE 
            , core::CategoryBits::HITBOX_SENSOR 
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    map->addChild(proj); 
}

void Axe::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(this->GetDamage());
    const auto projectile = m_extractor();
    const auto body = proj->AddPhysicsBody(projectile.size);
    proj->setPosition(projectile.origin);
    m_modifier(body);
    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::PROPS
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLAYER_PROJECTILE 
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    map->addChild(proj);
}

void Spear::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(this->GetDamage());
    const auto projectile = m_extractor();
    const auto body = proj->AddPhysicsBody(projectile.size);
    proj->setPosition(projectile.origin);
    m_modifier(body);
    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::PROPS
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLAYER_PROJECTILE 
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    map->addChild(proj);
}

void Bow::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(this->GetDamage());
    const auto projectile = m_extractor();
    auto body = proj->AddPhysicsBody(projectile.size);
    proj->setPosition(projectile.origin);
    proj->setContentSize(projectile.size);
    // push projectile
    m_modifier(body);
    const auto scaleFactor { 0.2f };
    const auto sprite = proj->AddImage("archer/library/arrow.png");
    sprite->setAnchorPoint({0.0f, 0.0f});
    sprite->setScale(scaleFactor);
    if (body->getVelocity().x > 0.f) {
        proj->FlipX();
    }

    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::PROPS
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLAYER_PROJECTILE
            , core::CategoryBits::PLATFORM
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    proj->SetLifetime(3.f);
    map->addChild(proj, 100); /// TODO: clean up this mess with Z-order!
}

void Legs::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");

    const auto proj = Projectile::create(this->GetDamage());
    map->addChild(proj, 100); /// TODO: clean up this mess with Z-order!

    const auto projectile = m_extractor();
    const auto body = cocos2d::PhysicsBody::createCircle(
        projectile.size.width / 2.f
        , cocos2d::PhysicsMaterial{ 0.1f, 0.2f, 0.7f }
    );
    body->setDynamic(true);
    // body->setMass(10.f);
    body->setGravityEnable(true);
    body->setRotationEnable(true);
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::PLATFORM
            , core::CategoryBits::BOUNDARY
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::PROPS
            , core::CategoryBits::PLAYER_PROJECTILE
        )
    );
    proj->setAnchorPoint({0.5f, 0.5f});
    proj->setPosition(projectile.origin + projectile.size / 2.f);
    proj->setContentSize(projectile.size);
    // push projectile
    m_modifier(body);
    const auto sprite = proj->AddImage("old_man/library/stone.png");
    sprite->setScale(0.15f);
    sprite->setAnchorPoint({0.f, 0.f});
    
    proj->SetLifetime(5.f);
    proj->addComponent(body);
}

void Fireball::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    const auto scaleFactor { 0.2f };
    
    const auto proj = Projectile::create(this->GetDamage());
    proj->AddAnimator("fireball");
    proj->setScale(scaleFactor);

    const auto projectile = m_extractor();
    const auto body = cocos2d::PhysicsBody::createBox(
        projectile.size
        , cocos2d::PhysicsMaterial{ 1.f, 0.0f, 0.0f }
        , { -projectile.size.width / 2.f, 0.f }
    );
    body->setDynamic(true);
    body->setGravityEnable(false);

    proj->setAnchorPoint({0.0f, 0.0f});
    proj->setContentSize(projectile.size);
    proj->setPosition(projectile.origin);
    // push projectile
    m_modifier(body);
    // ---------------
    if(body->getVelocity().x > 0.f) {
        proj->FlipX();
    }

    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::PROPS
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::ENEMY_PROJECTILE
            , core::CategoryBits::PLATFORM
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE)
    };
    body->setCollisionBitmask(0);
    body->setCategoryBitmask(categoryMask);
    body->setContactTestBitmask(testMask);
    
    proj->SetLifetime(5.f);
    proj->addComponent(body);
    map->addChild(proj, 101); /// TODO: clean up this mess with Z-order!
}

void SlimeShot::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    const auto scaleFactor { 0.2f };
    
    const auto proj = Projectile::create(this->GetDamage());
    proj->AddAnimator("slime_attack");
    proj->setScale(scaleFactor);

    const auto projectile = m_extractor();
    const auto body = cocos2d::PhysicsBody::createBox(
        projectile.size
        , cocos2d::PhysicsMaterial{ 1.f, 0.0f, 0.0f }
        , { -projectile.size.width / 2.f, projectile.size.height }
    );
    body->setDynamic(true);
    body->setGravityEnable(false);

    proj->setAnchorPoint({0.0f, 0.0f});
    proj->setContentSize(projectile.size);
    proj->setPosition(projectile.origin);
    // push projectile
    m_modifier(body);
    // ---------------
    if(body->getVelocity().x > 0.f) {
        proj->FlipX();
    }

    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::PROPS
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLAYER_PROJECTILE
            , core::CategoryBits::PLATFORM
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    body->setCollisionBitmask(0);
    body->setCategoryBitmask(categoryMask);
    body->setContactTestBitmask(testMask);
    
    proj->SetLifetime(4.f);
    proj->addComponent(body);
    map->addChild(proj, 101); /// TODO: clean up this mess with Z-order!
}

void Stake::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    
    const auto proj = Projectile::create(this->GetDamage());
    const auto projectile = m_extractor();
    auto body = proj->AddPhysicsBody(projectile.size);
    proj->setPosition(projectile.origin);
    proj->setContentSize(projectile.size);
    // push projectile
    m_modifier(body);
    const auto scaleFactor { 0.2f };
    const auto sprite = proj->AddImage("cannon/library/Asset 4.png");
    sprite->setAnchorPoint({0.0f, 0.0f});
    sprite->setScale(scaleFactor);
    if (body->getVelocity().x > 0.f) {
        proj->FlipX();
    }

    const auto testMask {
        Utils::CreateMask(
            core::CategoryBits::HITBOX_SENSOR
            , core::CategoryBits::PROPS
            , core::CategoryBits::BOUNDARY
            , core::CategoryBits::PLATFORM
            , core::CategoryBits::PLAYER_PROJECTILE
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    proj->SetLifetime(3.f);
    map->addChild(proj, 100); /// TODO: clean up this mess with Z-order!
}
