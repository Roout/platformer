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

#include <unordered_map>
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
    m_follower = std::make_unique<SmoothFollower>(this);
    m_inputHandler = std::make_unique<UserInputHandler>(this);
    
    return true;
}

void Player::AddWeapon() {
    // create weapon (it should be read from config)
    const auto damage { 25.f };
    const auto range { 60.f };
    const auto preparationTime { 0.f };
    const auto attackDuration { m_animator->GetDuration(Utils::EnumCast(State::ATTACK)) };
    const auto reloadTime { 0.1f };
    m_weapon = std::make_unique<Sword>(
        damage, 
        range, 
        preparationTime,
        attackDuration,
        reloadTime 
    );
};

std::string  Player::GetStateName(Player::State state) {
    static std::unordered_map<Player::State, std::string> mapped {
        { Player::State::IDLE, "idle" },
        { Player::State::ATTACK, "attack" },
        { Player::State::JUMP, "jump" },
        { Player::State::WALK, "walk" },
        { Player::State::DEATH, "death" }
    };
    auto it = mapped.find(state);
    return (it != mapped.cend()? it->second: "");        
}

void Player::AddAnimator() {
    Unit::AddAnimator();
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::ATTACK), "attack"),
        std::make_pair(Utils::EnumCast(State::DEATH), "death"),
        std::make_pair(Utils::EnumCast(State::IDLE), "idle"),
        std::make_pair(Utils::EnumCast(State::JUMP), "jump"),
        std::make_pair(Utils::EnumCast(State::WALK), "walk")
    });
}

void Player::AddPhysicsBody() {
    Unit::AddPhysicsBody();
    
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
    Unit::pause();
    if(!this->IsDead()) {
        // prevent to being called onExit() when the player is dead and is being detached!
        m_inputHandler->Reset();
    }
}

void Player::update(float dt) {
    cocos2d::Node::update(dt);
     
    this->UpdateDebugLabel();
    this->UpdateWeapon(dt);
    this->UpdatePosition(dt); 
    this->UpdateCurses(dt);
    this->UpdateState(dt);
    this->UpdateAnimation(); 
}

void Player::UpdateDebugLabel() noexcept {
    auto state = dynamic_cast<cocos2d::Label*>(this->getChildByName("state"));
    state->setString(this->GetStateName(m_currentState));
}

void Player::UpdateAnimation() {
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
    else if( m_currentState != m_previousState ) {
        int repeatTimes { dragonBones::Animator::INFINITY_LOOP };
        if( m_currentState == State::ATTACK || m_currentState == State::JUMP ) {
            repeatTimes = 1;
        }
        m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
    }
}

void Player::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    const auto velocity { this->getPhysicsBody()->getVelocity() };
    static constexpr float EPS { 0.00001f };

    if( m_health <= 0 ) {
        m_currentState = State::DEATH;
    }
    else if( m_weapon->IsAttacking() ) {
        m_currentState = State::ATTACK;
    }
    else if( !this->IsOnGround() ) {
        m_currentState = State::JUMP;
    } 
    else if( helper::IsEquel(velocity.x, 0.f, EPS) ) {
        m_currentState = State::IDLE;
    } 
    else {
        m_currentState = State::WALK;
    }
}