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
    // Defines how high can the body jump
    static constexpr float JUMP_HEIGHT { 255.f };     // up to 3 tiles
    // Defines how fast the body reach the max height by single jump 
    static constexpr float TIME_OF_APEX_JUMP { 0.30 };  // standart time
    // Calculate grvity base on defined constancts: height, time ( G = -H / (2*t*t) )
    static constexpr float GRAVITY { 
        -JUMP_HEIGHT / (2 * TIME_OF_APEX_JUMP * TIME_OF_APEX_JUMP) 
    };
    
    Movement(cocos2d::PhysicsBody * const body);
    
    ~Movement();

    /**
     * Each frame applies forces or impulses which are scheduled by Push/Move
     * @param dt may be unused
     */
    void Update(float dt) noexcept;
    
    /**
     *  Push the body with predefined impulses (for now 
     * there are 2 types of impulses: used to jump or fall down).
     * Can push only along Y-axis for now.
     *  Impulses can be adjusted by setting coefficients: x & y. 
     * 
     *  @param x provide impulse horizontal direction 
     * and coefficient (work as simple multiplier of impulse)
     *  @param y provide impulse vertical direction 
     * and coefficient (work as simple multiplier of impulse)
     * 
     * Expect: x, y = [-1.f, 1.f]
     */
    void Push(float x, float y) noexcept;

    /**
     *  Move the body with predefined forces in 4 directions.
     * Stop body if `x = 0.f, y = 0.f`.
     *  Forces like impulses can also be adjusted by setting coefficients: x & y. 
     * 
     *  @param x provide force horizontal direction 
     * and coefficient (work as simple multiplier of force)
     *  @param y provide force vertical direction 
     * and coefficient (work as simple multiplier of force)
     * 
     * Expect: x, y = [-1.f, 1.f]
     */
    void Move(float x, float y) noexcept;

    /**
     * Stop the movement along X-axis and reset forces 
     */
    void Stop() noexcept;
    
    void SetMaxSpeed(float speed) noexcept;

private:
    cocos2d::PhysicsBody * const m_body { nullptr };

    // jump speed calculated in runtime base on compile-time constants
    const float m_upJumpSpeed { 0.f };
    // fall speed calculated in runtime base on compile-time constants
    const float m_downJumpSpeed { 0.f };
    // max possible 
    const float m_maxVelocity { 1550.f };

    // set by user manually
    float m_desiredVelocity { 400.f };

    // The force scheduled to be applied to the body each update
    // can be reset by `Stop` function or overriden by the next `Move` call.
    // Used to move around like kinematic body and solve problem with 
    // body being `inserted into` and `pushed out of` another collidable body
    cocos2d::Vec2 m_force { 0.f, 0.f };

    // The impulse scheduled to be applied to the body NEXT update. 
    // Then it will be reseted to zero-vector
    cocos2d::Vec2 m_impulse { 0.f, 0.f };
};

#endif // UNIT_MOVEMENT_HPP
