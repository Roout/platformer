#include "Spider.hpp"
#include "Core.hpp"

#include "components/Path.hpp"
#include "components/Navigator.hpp"
#include "components/DragonBonesAnimator.hpp"
#include "components/Weapon.hpp"
#include "components/Movement.hpp"

#include "configs/JsonUnits.hpp"

#include <cmath>

namespace Enemies {

Spider* Spider::create(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Spider *model) 
{
    auto pRet { new (std::nothrow) Spider(id, contentSize, model) };
    if (pRet && pRet->init()) {
        pRet->autorelease();
    } 
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

Spider::Spider(size_t id
    , const cocos2d::Size& contentSize
    , const json_models::Spider *model
)
    : Bot { id, core::EntityNames::SPIDER }
    , m_model { model }
{
    m_contentSize = contentSize;
    m_physicsBodySize = cocos2d::Size{ contentSize.width * 0.89f, contentSize.height * 0.77f };
    m_hitBoxSize = cocos2d::Size{ contentSize.width, contentSize.height * 0.82f };

    // define size of the graphical content
    // m_contentSize = cocos2d::Size { 45.f, 65.f };
    // define size of the physics body
    // m_physicsBodySize = cocos2d::Size{ 40.f, 40.f };
    // m_hitBoxSize = cocos2d::Size{ 45.f, 45.f };
}

bool Spider::init() {
    if (!Bot::init()) {
        return false; 
    }
    m_health = m_model->health;
    m_movement->SetMaxSpeed(m_model->idleSpeed);
    // override content size because the body is with offset and smaller than the 
    // graph content
    setContentSize(m_contentSize);
    return true;
};

void Spider::onEnter() {
    cocos2d::Node::onEnter();
}

void Spider::onExit() {
    m_web = nullptr;
    cocos2d::Node::onExit();
}

void Spider::MoveAlong(Movement::Direction dir) noexcept {
    m_movement->Move(dir);
}


void Spider::CreateWebAt(const cocos2d::Vec2& start) {
    m_webStart = start;
    m_web = cocos2d::DrawNode::create(m_model->linewidth);
    getParent()->addChild(m_web, getLocalZOrder() - 1);
}

void Spider::UpdateWeb() {
    if (m_web) {
        m_web->clear();
        const auto shiftY { m_contentSize.height * 0.6f };
        auto assMiddle = getPosition() + cocos2d::Vec2{ 0.f, shiftY}; 
        m_web->drawLine(m_webStart, assMiddle, cocos2d::Color4F::WHITE);        
    }
}

void Spider::update(float dt) {
    cocos2d::Node::update(dt);
    // custom updates
    UpdateDebugLabel();
    // Spider's dead body still moves!!!
    UpdatePosition(dt);
    if (!IsDead()) {
        UpdateCurses(dt);
    }
    UpdateWeb(); 
    UpdateState(dt);
    UpdateAnimation(); 
};

void Spider::OnEnemyIntrusion() {
    m_detectEnemy = true;
    m_movement->SetMaxSpeed(m_model->alertSpeed);
};

void Spider::OnEnemyLeave() {
    m_detectEnemy = false;
    m_movement->SetMaxSpeed(m_model->idleSpeed);
};

/// Unique to warrior
void Spider::AttachNavigator(Path&& path) {
    m_navigator = std::make_unique<Navigator>(this, std::move(path));
    m_navigator->FollowPath();
};

void Spider::TryAttack() {
    // do nothing as it doens't attack
    assert(false && "Cannot attack!");
};

bool Spider::NeedAttack() const noexcept {
    assert(false && "Cannot attack!");
    return false; // spider doesn't attack at least for now
};

/// Bot interface

void Spider::UpdateState(const float dt) noexcept {
    m_previousState = m_currentState;

    if (m_health <= 0) {
        m_currentState = State::DEAD;
    }
    else {
        m_currentState = State::PATROL;
    }
};

void Spider::UpdatePosition(const float dt) noexcept {
    if (!IsDead()) {
        m_navigator->Update(dt);
    }
    m_movement->Update();
};

void Spider::UpdateAnimation() {
    if (m_currentState != m_previousState) {
        const auto isOneTimeAttack { m_currentState == State::DEAD };
        const auto repeatTimes { isOneTimeAttack ? 1 : dragonBones::Animator::INFINITY_LOOP };
        (void) m_animator->Play(Utils::EnumCast(m_currentState), repeatTimes);
        if (IsDead()) {
            OnDeath();
        }
    }
};

void Spider::OnDeath() {
    // Interface
    getChildByName("health")->removeFromParent();
    // Physics
    const auto body = getPhysicsBody();
    const auto hitBoxTag { Utils::EnumCast(core::CategoryBits::HITBOX_SENSOR) };
    const auto hitBoxSensor { body->getShape(hitBoxTag) };
    body->setGravityEnable(true);
    hitBoxSensor->setContactTestBitmask(0); // don't cause any damage to player
    // Movement
    m_movement->Stop(Movement::Axis::XY);
    m_movement->Push(Movement::Direction::DOWN, 0.1f);
    // Animation
    m_animator->EndWith([this]() {
        if (m_web) {
            m_web->removeFromParent();
            m_web = nullptr;
        }
        runAction(cocos2d::RemoveSelf::create(true));
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
    const auto yOffset { floorf(m_contentSize.height * 0.65f) };
    body->setPositionOffset({0.f, yOffset});
    body->setDynamic(true);
    body->setGravityEnable(false);
    body->setRotationEnable(false);

    // change masks for physics body
    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::ENEMY));
    body->setCollisionBitmask(0); // collide with nothing

    const auto hitBoxShape = cocos2d::PhysicsShapeBox::create(
        m_hitBoxSize 
        , cocos2d::PHYSICSSHAPE_MATERIAL_DEFAULT
        //, {0.f, yOffset}
    );
    hitBoxShape->setSensor(true);
    const auto tag { Utils::CreateMask(core::CategoryBits::HITBOX_SENSOR) };
    hitBoxShape->setTag(tag);
    hitBoxShape->setCategoryBitmask(tag);
    hitBoxShape->setCollisionBitmask(0);
    hitBoxShape->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::TRAP
            , core::CategoryBits::PLAYER_PROJECTILE
            , core::CategoryBits::HITBOX_SENSOR
        )
    );
    body->addShape(hitBoxShape, false);
    addComponent(body);
};

void Spider::AddAnimator() {
    Unit::AddAnimator();
    m_animator->setScale(0.2f); // override scale
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::PATROL),  GetStateName(State::PATROL)),
        std::make_pair(Utils::EnumCast(State::DEAD),    GetStateName(State::DEAD))
    });
};

} // namespace Enemies 
