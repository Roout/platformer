#include "Weapon.hpp"
#include "Projectile.hpp"
#include "Utils.hpp"
#include "Player.hpp"
#include "Unit.hpp"
#include "Core.hpp"
#include "units/FireCloud.hpp"

#include <string>

using namespace std::literals;

/**
 * TODO: 
 * - [ ] remove run-time dependency on UI. It shouldn't be attached to the scene here
 * - [ ] remove code repetition: everything except mask is the same!
 * - [ ] cleanup mess with z-order
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
    map->addChild(proj, 100); 
}

void Legs::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");

    const auto proj = Projectile::create(this->GetDamage());
    map->addChild(proj, 100); 

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
        Utils::CreateMask(core::CategoryBits::PLATFORM, core::CategoryBits::BOUNDARY)
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

void PlayerFireball::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    const auto scaleFactor { 0.2f };
    
    const auto proj = Projectile::create(this->GetDamage());
    proj->AddAnimator("mc_fireball", "mc/mc_fireball");
    proj->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Projectile::State::IDLE), "walk"),       // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_PLAYER), "attack"),  // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_GROUND), "attack")  // sorry the illustrator is a little bit of an idiot
    });
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
    map->addChild(proj, 101); 
}

// READY -> PREPARATION -> [ ATTACK -> DELAY -> ATTACK ] -> RELOAD -> READY ... 
void BossFireball::UpdateState(const float dt) noexcept {
    if(m_state != State::READY) {
        m_timer -= dt;
        if(m_timer <= 0.f) {
            this->NextState();
            if(m_state == State::ATTACK) {
                this->OnAttack();
                m_delay = DELAY;
                m_attackedTwice = false;
            }
        } 
        else if(m_state == State::ATTACK) { // timer > 0
            m_delay -= dt;
            if(m_delay <= 0.f && !m_attackedTwice) {
                this->OnAttack();
                m_delay = DELAY;
                m_attackedTwice = true;
            }
        }
    }
}

void BossFireball::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    if(!level) return;
    const auto map = level->getChildByName("Map");
    if(!map) return;
    const auto boss = map->getChildByName<Unit*>(core::EntityNames::BOSS);
    if(!boss || boss->IsDead()) return;

    const auto scaleFactor { 0.15f };
    
    const auto proj = Projectile::create(this->GetDamage());
    proj->AddAnimator("fireball", "boss/boss_fireball");
    proj->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Projectile::State::IDLE),        "walk"),       // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_PLAYER),  "attack_2"),   // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_GROUND),  "attack_1")   // sorry the illustrator is a little bit of an idiot
    });
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
    if(body->getVelocity().x > 0.f) proj->FlipX();

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
    
    proj->SetLifetime(5.f);
    proj->addComponent(body);
    map->addChild(proj, 101); 
}

void BossChain::OnAttack() {
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
            , core::CategoryBits::PLAYER_PROJECTILE 
            , core::CategoryBits::HITBOX_SENSOR 
        )
    };
    const auto categoryMask {
        Utils::CreateMask(core::CategoryBits::ENEMY_PROJECTILE)
    };
    proj->SetCategoryBitmask(categoryMask);
    proj->SetContactTestBitmask(testMask);
    map->addChild(proj); 
}

void PlayerSpecial::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    const auto scaleFactor { 0.17f };
    
    const auto proj = Projectile::create(this->GetDamage());
    proj->AddAnimator("mc_special", "mc/mc_special");
    proj->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Projectile::State::IDLE), "walk"),       // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_PLAYER), "attack"),  // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_GROUND), "attack")  // sorry the illustrator is a little bit of an idiot
    });
    proj->setScale(scaleFactor);

    const auto projectile = m_extractor();
    const auto body = cocos2d::PhysicsBody::createBox(
        projectile.size
        , cocos2d::PhysicsMaterial{ 1.f, 0.0f, 0.0f }
        , { -projectile.size.width / 2.f, projectile.size.height * 0.2f }
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
    map->addChild(proj, 101); 
}

void SlimeShot::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    const auto scaleFactor { 0.2f };
    
    const auto proj = Projectile::create(this->GetDamage());
    proj->AddAnimator("slime_attack", "slime_attack");
    proj->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Projectile::State::IDLE), "walk"),       // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_PLAYER), "attack"),  // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_GROUND), "attack")  // sorry the illustrator is a little bit of an idiot
    });
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
    map->addChild(proj, 101); 
}

void BossFireCloud::OnAttack() {
    // TODO: generate a fire cloud
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    const auto boss = map->getChildByName<Unit*>(core::EntityNames::BOSS);

    const auto form = m_extractor();
    
    // size_t id, const cocos2d::Size& contentSize
    auto cloud = Enemies::FireCloud::create(1, form.size);
    if(!boss->IsLookingLeft()) cloud->Turn();

    // push body up
    auto body = cloud->getPhysicsBody();
    m_modifier(body);

    cloud->setPosition(form.origin);
    map->addChild(cloud, 101);
}

void CloudFireball::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");
    const auto scaleFactor { 0.2f };
    
    const auto proj = Projectile::create(this->GetDamage());
    proj->AddAnimator("fire", "boss/boss_attack");
    proj->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Projectile::State::IDLE),        "walk"),      // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_PLAYER),  "attack_2"),  // sorry the illustrator is a little bit of an idiot
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_GROUND),  "attack_1")   // sorry the illustrator is a little bit of an idiot
    });
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
            , core::CategoryBits::PLAYER_PROJECTILE
            // , core::CategoryBits::PLATFORM
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
    map->addChild(proj, 101); 
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
    map->addChild(proj, 100); 
}

StalactitePart::StalactitePart(
    float damage
    , float range
    , float preparationTime 
    , float attackTime 
    , float reloadTime
    , size_t index
)   
    : Weapon { damage, range, preparationTime, attackTime, reloadTime }
    , m_index { index }
{}

void StalactitePart::OnAttack() {
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    const auto level = runningScene->getChildByName("Level");
    const auto map = level->getChildByName("Map");

    const auto proj = Projectile::create(this->GetDamage());
    auto name   = cocos2d::StringUtils::format("%s_%d_projectile", core::EntityNames::STALACTITE, m_index);
    auto prefix = cocos2d::StringUtils::format("stalactites/%s_%d/%s", core::EntityNames::STALACTITE, m_index, name.c_str());
    proj->AddAnimator(name, prefix);
    proj->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Projectile::State::IDLE), "attack"),
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_PLAYER), "dead"),
        std::make_pair(Utils::EnumCast(Projectile::State::HIT_GROUND), "dead")
    });
    const auto scaleFactor { 0.095f };
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
    
    proj->SetLifetime(5.f);
    proj->addComponent(body);
    map->addChild(proj, 101); 
}