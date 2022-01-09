#ifndef PROPS_HPP
#define PROPS_HPP

#include "cocos2d.h"
#include "Utils.hpp"

#include <string_view>
#include <cstdint>

namespace dragonBones {
    class Animator;
}

namespace props {

// Contains all possible porps
enum class Name : uint8_t { 
    BARREL_B = 0,
    BARREL_S,
    BOX,
    BUCKET,
    CHEST,

    COUNT, 
    UNDEFINED
};

Name GetPropName(const std::string& name) noexcept;

std::string_view GetPropName(Name name) noexcept;

class Prop : public cocos2d::Node {
public:
    static Prop * create(Name name, const cocos2d::Size& size, float scale) noexcept;

    [[nodiscard]] bool init() override;
   
    void pause() override;
    
    void resume() override;

    void Explode() noexcept;

private:

    Prop(Name name, const cocos2d::Size& size, float scale);

    void AddAnimator();

    void AddPhysicsBody();

private:
    enum class State: uint8_t {
        IDLE,
        DEAD
    };

    State m_state { State::IDLE };
    Name m_name { Name::UNDEFINED };
    dragonBones::Animator * m_animator { nullptr };
    cocos2d::Size m_originContentSize {};
    float m_scale {0.f};
};


}

#endif // PROPS_HPP
