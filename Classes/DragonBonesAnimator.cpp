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

        // TODO: scale factor depends on device resolution so it can'be predefined constant.
        constexpr auto designedScaleFactor { 0.2f };
        const auto adjustedScaleFactor { 
            SizeDeducer::GetInstance().GetAdjustedSize(designedScaleFactor) 
        };
        m_armatureDisplay->setScale(adjustedScaleFactor);
        this->setContentSize(
            SizeDeducer::GetInstance().GetAdjustedSize(m_armatureDisplay->getBoundingBox().size)
        );

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

    void Animator::FlipX() {
        const auto armature = m_armatureDisplay->getArmature();
        armature->setFlipX(!armature->getFlipX());
    }

    Animator& Animator::Play(std::size_t state, int times) {
        m_lastAnimationState = m_armatureDisplay->getAnimation()->play(m_animations.at(state), times);
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