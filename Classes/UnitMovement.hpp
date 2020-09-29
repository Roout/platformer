#ifndef UNIT_MOVEMENT_HPP
#define UNIT_MOVEMENT_HPP

#include "cocos/math/Vec2.h"
#include "Utils.hpp"

namespace cocos2d {
    class PhysicsBody;
}

/**
 * Wrapper around physics body.
 * Provide interface for simple movement 
 */
class Movement final {
public:
    static constexpr float m_jumpHeight { 255.f };     // up to 3 tiles
    static constexpr float m_timeToJumpApex { 0.30 };  // standart time
    static constexpr float m_gravity { 
        -m_jumpHeight / (2 * m_timeToJumpApex * m_timeToJumpApex) 
    };
    
    Movement(cocos2d::PhysicsBody * const body);
    
    ~Movement();

    void Update(float dt) noexcept;
    
    void Push(float x, float y) noexcept;

    void Move(float x, float y) noexcept;

    void Stop() noexcept;
    
    void SetMaxSpeed(float speed) noexcept;

private:
    cocos2d::PhysicsBody * const m_body { nullptr };

    const float m_maxVelocity { 1550.f };
    const float m_upJumpSpeed { 0.f };
    const float m_downJumpSpeed { 0.f };
    
    float m_desiredVelocity { 550.f };

    cocos2d::Vec2 m_force { 0.f, 0.f };

    cocos2d::Vec2 m_impulse { 0.f, 0.f };
};

#endif // UNIT_MOVEMENT_HPP
