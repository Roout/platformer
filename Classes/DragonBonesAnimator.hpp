#ifndef DRAGON_BONES_ANIMATOR_HPP
#define DRAGON_BONES_ANIMATOR_HPP

#include <unordered_map>
#include <string>
#include <optional>
#include <initializer_list>
#include <functional>

#include "cocos2d.h"

namespace dragonBones {
    class CCArmatureDisplay;
    class AnimationState;
}

/** 
 * - [x] build or get from the cache CCArmatureDisplay object on construction
 * - [x] have cocos2d::Node interface
 * - [x] define a size which will be later used constructing physics body
 * - [x] switch between states: animator.play(State::idle)
 * - [x] add completion handler
 * - [x] pause, resume, flip
 */
namespace dragonBones {

    class Animator : public cocos2d::Node {
    public:
        static constexpr int INFINITY_LOOP { 0 };

        static Animator * create(std::string&& armatureCacheName);

        bool init() override;

        void update(float [[maybe_unused]] dt) override;

        void pause() override;

        void resume() override;

        void FlipX();

        Animator& Play(std::size_t state, int times);

        void EndWith(std::function<void()>&& handler);

        void InitializeAnimations(std::initializer_list<std::pair<std::size_t, std::string>> animations);

    private:

        Animator(std::string&& armatureCacheName) noexcept;

    private:
        CCArmatureDisplay *m_armatureDisplay { nullptr };
        AnimationState *m_lastAnimationState { nullptr };
        std::optional<std::function<void()>> m_completionHandler {};

        std::string m_armatureName;
        std::unordered_map<std::size_t, std::string> m_animations {};
    };
}

#endif // DRAGON_BONES_ANIMATOR_HPP