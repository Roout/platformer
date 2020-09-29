#include "Spider.hpp"
#include "Path.hpp"
#include "Navigator.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"
#include "Weapon.hpp"
#include "UnitMovement.hpp"

namespace Enemies {

Spider* Spider::create(size_t id) {
    auto pRet { new (std::nothrow) Spider(id, core::EntityNames::SPIDER) };
    if( pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Spider::Spider(size_t id, const char* dragonBonesName) :
    Bot{ id, dragonBonesName }
{
    // define size of the graphical content
    m_designedSize = cocos2d::Size { 90.f, 125.f };
    // define size of the physics body
    m_physicsBodySize = cocos2d::Size{ 80.f, 80.f };
}

bool Spider::init() {
    if(!Bot::init()) {
        return false; 
    }
    m_movement->SetMaxSpeed(130.f);
    // override content size because the body is with offset and smaller than the 
    // graph content
    this->setContentSize(m_designedSize);
    return true;
};

void Spider::onEnter() {
    cocos2d::Node::onEnter();
}

void Spider::onExit() {
    m_web = nullptr;
    cocos2d::Node::onExit();
}

void Spider::MoveAlong(float x, float y) noexcept {
    m_movement->Move(x, y);
}


void Spider::CreateWebAt(const cocos2d::Vec2& start) {
    m_webStart = start;
    
    constexpr float lineWidth { 25.f }; 
    m_web = cocos2d::DrawNode::create(lineWidth);
    this->getParent()->addChild(m_web, this->getLocalZOrder() - 1);
}

void Spider::UpdateWeb() {
    if(m_web) {
        m_web->clear();
        const auto shiftY { m_designedSize.height * 0.6f };
        auto middleOfAss = this->getPosition() + cocos2d::Vec2{ 0.f, shiftY}; 
        m_web->drawLine(m_webStart, middleOfAss, cocos2d::Color4F::WHITE);        
    }
}

void Spider::update(float dt) {
    cocos2d::Node::update(dt);
    // custom updates
    this->UpdateDebugLabel();
    this->UpdatePosition(dt);
    this->UpdateWeb(); 
    this->UpdateCurses(dt);
    this->UpdateState(dt);
    this->UpdateAnimation(); 
};

void Spider::OnEnemyIntrusion() {
    m_detectEnemy = true;
    m_movement->SetMaxSpeed(150.f);
};

void Spider::OnEnemyLeave() {
    m_detectEnemy = false;
    m_movement->SetMaxSpeed(80.f);
};

/// Unique to warrior
void Spider::AttachNavigator(Path&& path) {
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    m_navigator->FollowPath();
};

void Spider::TryAttack() {
    // do nothing as it doens't attack
};

bool Spider::NeedAttack() const noexcept{
    return false; // spider doesn't attack at least for now
};

/// Bot interface

void Spider::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if( m_health <= 0 ) {
        m_currentState = State::DEAD;
    }
    else {
        m_currentState = State::PATROL;
    }
};

void Spider::UpdatePosition(const float dt) noexcept {
    if(!this->IsDead()) {
        m_navigator->Update(dt);
    }
    m_movement->Update(dt);
};

void Spider::UpdateAnimation() {
    if(m_currentState != m_previousState) {
        const auto isOneTimeAttack { 
            m_currentState == State::DEAD
        };
        const auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if(this->IsDead()) {
            this->OnDeath();
        }
    }
};

void Spider::OnDeath() {
    this->getChildByName("health")->removeFromParent();
    const auto body = this->getPhysicsBody();
    body->setGravityEnable(true);
    body->setContactTestBitmask(0); // don't cause any damage to player
    m_movement->Stop(); // reset forces
    m_movement->Push(0.f, -0.1f); // push down
    m_animator->EndWith([this]() {
        if(this->m_web) {
            this->m_web->removeFromParent();
            this->m_web = nullptr;
        }
        this->runAction(cocos2d::RemoveSelf::create(true));
    });
};

void Spider::AddPhysicsBody() {
    // NOTE: instead of using Unit::AddPhysicsBody I ended with own `body`
    // as there is no need to create a sensor or other shapes
    const auto body = cocos2d::PhysicsBody::createBox(
        m_physicsBodySize,
        cocos2d::PhysicsMaterial(1.f, 0.f, 0.2f)
    );
    // explicitly define the offset rather than in constructor of the body
    // because in constructor offset is added to the shape!
    const auto yOffset { m_designedSize.height * 0.65f };
    body->setPositionOffset({0.f, yOffset});
    body->setMass(25.f);
    body->setDynamic(true);
    body->setGravityEnable(false);
    body->setRotationEnable(false);

    // change masks for physics body
    body->setCategoryBitmask(
        Utils::CreateMask(core::CategoryBits::ENEMY)
    );
    body->setCollisionBitmask(0); // collide with nothing
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::TRAP,
            core::CategoryBits::PROJECTILE,
            core::CategoryBits::ENEMY,
            core::CategoryBits::HERO
        )
    );
    this->addComponent(body);
};

void Spider::AddAnimator() {
    Unit::AddAnimator();
    m_animator->setScale(0.4f); // override scale
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::PATROL),  GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEAD),    GetStateName(State::DEAD))
    });
};

} // namespace Enemies 
