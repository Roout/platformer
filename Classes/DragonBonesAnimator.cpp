#include "DragonBonesAnimator.hpp"
#include "Utils.hpp"
#include "ResourceManagement.hpp"
#include "SizeDeducer.hpp"

namespace dragonBones {

    Animator * Animator::create(std::string&& armatureCacheName) {
        auto pRet = new (std::nothrow) Animator(std::move(armatureCacheName));
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
        m_armatureDisplay = Resource::BuildArmatureDisplay(m_armatureName); 
        this->addChild(m_armatureDisplay);
        return true;
    }

    void Animator::update(float [[maybe_unused]] dt) {
        if(m_lastAnimationState && 
            m_lastAnimationState->isCompleted() &&
            m_completionHandler.has_value()
        ) {
            (*m_completionHandler)();
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
        m_completionHandler.emplace(std::move(handler));
    }

    void Animator::InitializeAnimations(std::initializer_list<std::pair<std::size_t, std::string>> animations) {
        m_animations.reserve(std::size(animations));
        for(auto&& value: animations) {
            m_animations.emplace(std::move(value));
        }
    }

    Animator::Animator(std::string&& armatureCacheName) noexcept 
        : m_armatureName { std::move(armatureCacheName) }
    {
    }

}