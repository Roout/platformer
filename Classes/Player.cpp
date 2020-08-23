#include "Player.hpp"
#include "SmoothFollower.hpp"
#include "PhysicsHelper.hpp"
#include "Utils.hpp"
#include "Core.hpp"
#include "SizeDeducer.hpp"
#include "Weapon.hpp"
#include "DeathScreen.hpp"

Player* Player::create(const cocos2d::Size& size) {
    auto pRet { new (std::nothrow) Player(size) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Player::Player(const cocos2d::Size& sz) :
    Unit { sz, "mc" }
{
    // Create weapon
    const int damage { 25 };
    const int range { SizeDeducer::GetInstance().GetAdjustedSize(60) };
    const float reloadTime { m_maxAttackTime };
    m_weapon = std::make_unique<Sword>( damage, range, reloadTime );
}


bool Player::init() {
    if( !Unit::init()) {
        return false; 
    }
    m_follower = std::make_unique<SmoothFollower>(this);

    const auto body { this->getPhysicsBody() };
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::HERO)
    );
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY, 
            core::CategoryBits::PLATFORM 
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::PROJECTILE,
            core::CategoryBits::TRAP,
            core::CategoryBits::PLATFORM
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
    return true;
}

void Player::setPosition(const cocos2d::Vec2& position) {
    Node::setPosition(position.x, position.y);
    m_follower->Reset();
}

void Player::update(float dt) {
    this->UpdateDebugLabel();
    this->UpdateWeapon(dt);
    this->UpdatePosition(dt); 
    m_follower->UpdateMapPosition(dt);
    this->UpdateCurses(dt);
    this->UpdateAnimation(); 
    this->UpdateState(dt);
}

void Player::UpdateAnimation() {
    if( m_currentState.m_act == Act::dead ) {
        const auto emitter = cocos2d::ParticleSystemQuad::create("particle_texture.plist");
        emitter->setAutoRemoveOnFinish(true);
        /// TODO: adjust for the multiresolution
        emitter->setScale(0.4f);
        emitter->setPositionType(cocos2d::ParticleSystem::PositionType::RELATIVE);
        emitter->setPosition(this->getPosition());
        this->getParent()->addChild(emitter, 9);
    } 
    else {
        Unit::UpdateAnimation();
    }
}

void Player::UpdateState(const float dt) noexcept {
    if( m_currentState.m_act == Act::dead ) {
        m_follower.reset();
        this->removeFromParent();
        // show death screen:
        const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();

        const auto node = DeathScreen::create();
        node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        node->setPosition(origin / 2.f + visibleSize / 2.f );

        const auto scene = cocos2d::Director::getInstance()->getRunningScene();
        const auto ui = scene->getChildByName("Interface");
        ui->addChild(node);

        return;
    }

    m_previousState = m_currentState;

    constexpr float EPS { 0.00001f };

    // update character state
    if( m_currentState.m_act == Act::attack ) {
        m_attackTime -= dt;
        if( helper::IsPositive(m_attackTime, EPS) ) {
            // exit to not change an attack state
            return; 
        }
    }

    const auto velocity { this->getPhysicsBody()->getVelocity() };

    if( !this->IsOnGround() ) {
        m_currentState.m_act = Act::jump;
    } else if( helper::IsEquel(velocity.x, 0.f, EPS) ) {
        m_currentState.m_act = Act::idle;
    } else {
        m_currentState.m_act = Act::move;
    }
}