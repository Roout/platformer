#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"

#include "DragonBonesAnimator.hpp"
#include "Utils.hpp"

namespace dragonBones {

    Animator * Animator::create(std::string&& prefix, std::string&& armatureCacheName) {
        auto pRet = new (std::nothrow) Animator(std::move(prefix), std::move(armatureCacheName));
        if(pRet && pRet->init()) {
            pRet->autorelease();
        }
        else {
            delete pRet;
            pRet = nullptr;
        }
        return pRet;
    }

    bool Animator::init() {
        if(!cocos2d::Node::init()) {
            return false;
        }
        this->scheduleUpdate();
        m_armatureDisplay = BuildArmatureDisplay(); 
        this->addChild(m_armatureDisplay);
        return true;
    }

    void Animator::update(float [[maybe_unused]] dt) {
        if(m_lastAnimationState && 
            m_lastAnimationState->isCompleted() &&
            m_completionHandler
        ) {
            m_completionHandler();
        }
    }

    void Animator::pause() {
        cocos2d::Node::pause();
        if(m_lastAnimationState && m_lastAnimationState->isPlaying()) {
            m_lastAnimationState->stop();
        }
    }

    void Animator::resume() {
        cocos2d::Node::resume();
        if(m_lastAnimationState && !m_lastAnimationState->isPlaying()) {
            m_lastAnimationState->play();
        }
    }

    bool Animator::IsPlaying() const noexcept {
        return m_lastAnimationState && m_lastAnimationState->isPlaying();
    }

    bool Animator::IsPlaying(std::size_t type) const noexcept {
        return this->IsPlaying() && type == m_lastAnimationId;
    }

    float Animator::GetDuration(std::size_t type) const noexcept {
        const auto& name = m_animations.at(type);
        const auto& animations = m_armatureDisplay->getAnimation()->getAnimations();
        const auto data = animations.at(name);
        return (data? data->duration : 0.f);
    }

    void Animator::FlipX() {
        const auto armature = m_armatureDisplay->getArmature();
        armature->setFlipX(!armature->getFlipX());
    }

    Animator& Animator::Play(std::size_t id, int times) {
        m_lastAnimationId = id;
        m_lastAnimationState = m_armatureDisplay->getAnimation()->play(m_animations.at(id), times);
        return *this;
    }

    void Animator::EndWith(std::function<void()>&& handler) {
        m_completionHandler = std::move(handler);
    }

    void Animator::InitializeAnimations(std::initializer_list<std::pair<std::size_t, std::string>> animations) {
        m_animations.reserve(std::size(animations));
        for(auto&& value: animations) {
            m_animations.emplace(std::move(value));
        }
    }

    void Animator::AddAnimation(std::pair<std::size_t, std::string> && animation) {
        m_animations.emplace(std::move(animation));
    }


    Animator::Animator(std::string&& prefix, std::string&& armatureCacheName) noexcept 
        : m_armatureName { std::move(armatureCacheName) }
        , m_prefix{ std::move(prefix) }
    {
    }

    CCArmatureDisplay* Animator::BuildArmatureDisplay() const {
        const auto factory = CCFactory::getFactory();
        std::string path = m_prefix.empty()? "" : m_prefix + "/";
        if(const auto bonesData = factory->getDragonBonesData(m_armatureName); bonesData == nullptr) {    
            factory->loadDragonBonesData( path + "/" + m_armatureName + "_ske.json");
        }
        if(const auto texture = factory->getTextureAtlasData(m_armatureName); texture == nullptr) {
            factory->loadTextureAtlasData( path + "/" + m_armatureName + "_tex.json");
        }
        return factory->buildArmatureDisplay("Armature", m_armatureName);
    }

}