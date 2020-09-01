#include "Player.hpp"
#include "SmoothFollower.hpp"
#include "UserInputHandler.hpp"
#include "PhysicsHelper.hpp"
#include "Utils.hpp"
#include "Core.hpp"
#include "SizeDeducer.hpp"
#include "Weapon.hpp"
#include "DeathScreen.hpp"
#include "DragonBonesAnimator.hpp"

#include <algorithm>

Player* Player::create() {
    auto pRet { new (std::nothrow) Player() };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Player::Player() :
    Unit { "mc" }
{
    m_designedSize = cocos2d::Size{ 80.f, 135.f };
}

bool Player::init() {
    if( !Unit::init()) {
        return false; 
    }
    // create weapon (it should be read from config)
    const auto damage { 25.f };
    const auto range { 60.f };
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(Act::attack)) };
    const auto reloadTime { 0.1f };
    m_weapon = std::make_unique<Sword>(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
    
    m_follower = std::make_unique<SmoothFollower>(this);
    m_inputHandler = std::make_unique<UserInputHandler>(this);
    
    return true;
}

void Player::AddAnimator() {
    std::string chachedArmatureName = m_dragonBonesName;
    m_animator = dragonBones::Animator::create(std::move(chachedArmatureName));
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(Act::attack), "attack"),
        std::make_pair(Utils::EnumCast(Act::dead), "idle"),
        std::make_pair(Utils::EnumCast(Act::idle), "idle"),
        std::make_pair(Utils::EnumCast(Act::jump), "jump"),
        std::make_pair(Utils::EnumCast(Act::move), "walk")
    });
    
    this->addChild(m_animator);
    m_animator->setScale(0.2f); // TODO: introduce multi-resolution scaling
}

void Player::AddPhysicsBody(const cocos2d::Size& size) {
    Unit::AddPhysicsBody(size);
    
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
}

void Player::setPosition(const cocos2d::Vec2& position) {
    Node::setPosition(position.x, position.y);
    m_follower->Reset();
}

void Player::UpdatePosition(const float dt) noexcept {
    if(!this->IsDead()) {
        m_movement->Update(dt);
        m_follower->UpdateMapPosition(dt);
    }
}

void Player::pause() {
    cocos2d::Node::pause();
    m_animator->pause();
    if(!this->IsDead()) {
        // prevent to being called onExit() when the player is dead and is being detached!
        m_inputHandler->Reset();
    }
}

void Player::resume() {
    cocos2d::Node::resume();
    m_animator->resume();
}

void Player::update(float dt) {
    this->UpdateDebugLabel();
    this->UpdateWeapon(dt);
    this->UpdatePosition(dt); 
    this->UpdateCurses(dt);
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

void Player::UpdateAnimation() {
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

        // create a death screen
        const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();

        const auto node = DeathScreen::create();
        node->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        node->setPosition(visibleSize / 2.f);

        const auto ui = cocos2d::Director::getInstance()->getRunningScene()->getChildByName("Interface");
        ui->addChild(node);

        // remove from screen
        this->runAction(cocos2d::RemoveSelf::create());
    } 
    else if( m_currentState.m_act != m_previousState.m_act ) {
        int repeatTimes { dragonBones::Animator::INFINITY_LOOP };
        if( m_currentState.m_act == Act::attack || 
            m_currentState.m_act == Act::jump 
        ) {
            repeatTimes = 1;
        }
        m_animator->Play(Utils::EnumCast(m_currentState.m_act), repeatTimes);
    }
}

void Player::UpdateState(const float dt) noexcept {
    m_previousState.m_act = m_currentState.m_act;

    const auto velocity { this->getPhysicsBody()->getVelocity() };
    static constexpr float EPS { 0.00001f };

    if( m_health <= 0 ) {
        m_currentState.m_act = Act::dead;
    }
    else if( m_weapon->IsAttacking() ) {
        m_currentState.m_act = Act::attack;
    }
    else if( !this->IsOnGround() ) {
        m_currentState.m_act = Act::jump;
    } 
    else if( helper::IsEquel(velocity.x, 0.f, EPS) ) {
        m_currentState.m_act = Act::idle;
    } 
    else {
        m_currentState.m_act = Act::move;
    }
}