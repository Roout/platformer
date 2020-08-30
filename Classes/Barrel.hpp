#ifndef BARREL_HPP
#define BARREL_HPP

#include "cocos2d.h"
#include <cstdint> // std::uint8_t

namespace dragonBones {
    class Animator;
}

class Barrel final : public cocos2d::Node {
public:

    static Barrel * create();

    bool init() override;

    void pause() override;
    
    void resume() override;

    void Explode();

private:
    
    enum class State : std::uint8_t {
        idle = 0,
        exploded,
        COUNT
    };

    dragonBones::Animator * m_animator { nullptr };

private:

    Barrel() = default;

    void AddPhysicsBody(const cocos2d::Size& size);
};

#endif // BARREL_HPP