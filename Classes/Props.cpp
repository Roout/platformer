#include "Props.hpp"
#include "DragonBonesAnimator.hpp"
#include "Core.hpp"

#include <array>
#include <cassert>
#include <unordered_map>

using namespace std::literals;

namespace props {
    
Name GetPropName(const std::string& prop) noexcept {
    Name name { Name::UNDEFINED };
    using namespace std::literals;

    const std::array<Name, Utils::EnumSize<Name>()> names {
        Name::BARREL_B,
        Name::BARREL_S,
        Name::BOX,
        Name::BUCKET,
        Name::CHEST
    };
    const std::array<std::string_view, Utils::EnumSize<Name>()> repres {
        "barrel_b"sv,
        "barrel_s"sv,
        "box"sv,
        "bucket"sv,
        "chest"sv
    };
    
    for(size_t i = 0; i < names.size(); i++) {
        if (prop == repres[i]) {
            name = names[i];
            break;
        }
    }
    // assert(name != Name::UNDEFINED && "Can't find the given prop!");
    return name;
}

std::string_view GetPropName(Name name) noexcept {
    static const std::unordered_map<Name, std::string_view> repres {
          { Name::BARREL_B, "barrel_b"sv }
        , { Name::BARREL_S, "barrel_s"sv }
        , { Name::BOX,      "box"sv }
        , { Name::BUCKET,   "bucket"sv }
        , { Name::CHEST,    "chest"sv }
    };
    return repres.at(name);    
}

Prop * Prop::create(Name name, const cocos2d::Size& size, float scale) noexcept {
    auto pRet = new (std::nothrow) Prop(name, size, scale);
    if(pRet && pRet->init()) {
        pRet->autorelease();
    }    
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool Prop::init() {
    if(!cocos2d::Node::init()) {
        return false;
    }
    this->scheduleUpdate();

    // add animator
    this->AddAnimator();

    // add physics body 
    this->AddPhysicsBody();
    return true;
}

void Prop::pause() {
    cocos2d::Node::pause(); 
    m_animator->pause();
}

void Prop::resume() {
    cocos2d::Node::resume();
    m_animator->resume();
}

Prop::Prop(Name name, const cocos2d::Size& size, float scale) 
    : m_name { name }
    , m_originContentSize { size }
    , m_scale { scale }
{
}

void Prop::Explode() noexcept {
    if(m_state != State::DEAD) {
        m_state = State::DEAD;
        m_animator->Play(Utils::EnumCast(State::DEAD), 1).EndWith([this]() {
            this->runAction(cocos2d::RemoveSelf::create(true));
        });
        this->removeComponent(this->getPhysicsBody());
    }
}

void Prop::AddPhysicsBody() {
    auto physicsBodySize = m_originContentSize * m_scale;
    const auto body = cocos2d::PhysicsBody::createBox(
        physicsBodySize
        , cocos2d::PhysicsMaterial(1.f, 0.1f, 0.1f)
        , { -physicsBodySize.width / 2.f, 0.f } //floorf(physicsBodySize.height / 2.f)}
    );

    body->setDynamic(false);
    body->setGravityEnable(false);
    body->setRotationEnable(false);

    body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::PROPS));
    body->setCollisionBitmask(
        Utils::CreateMask(
            core::CategoryBits::BOUNDARY 
            , core::CategoryBits::PLATFORM
        )
    );
    body->setContactTestBitmask(
        Utils::CreateMask(
            core::CategoryBits::ENEMY_PROJECTILE
            , core::CategoryBits::PLAYER_PROJECTILE
        )
    );
    this->addComponent(body);
}
 
void Prop::AddAnimator() {
    std::string name(GetPropName(m_name));
    std::string prefix = "Map/props/"s + name;
    m_animator = dragonBones::Animator::create(std::move(prefix), std::move(name));
    m_animator->InitializeAnimations({
        std::make_pair(Utils::EnumCast(State::IDLE), "idle"),
        std::make_pair(Utils::EnumCast(State::DEAD), "dead")
    });
    (void) m_animator->Play(Utils::EnumCast(State::IDLE), dragonBones::Animator::INFINITY_LOOP);
    this->addChild(m_animator);
        
    // define size
    m_animator->setScale(m_scale);
    this->setContentSize(m_originContentSize * m_scale);
}

}