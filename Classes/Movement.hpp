#ifndef MOVEMENT_HPP
#define MOVEMENT_HPP

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

    enum class Direction : std::uint8_t {
        UP, DOWN, LEFT, RIGHT
    };

    enum class Axis : std::uint8_t {
        X, Y, XY
    };

    Movement(cocos2d::PhysicsBody * const body
        , float gravity
        , float jumpHeight
    );
    
    Movement(Movement const&) = delete;
    Movement& operator= (Movement const&) = delete;

    Movement(Movement&&) = delete;
    Movement& operator= (Movement&&) = delete;

    ~Movement();

    /**
     * TODO: add delta time to prevent different movement speed for differenrt FPS
     * 
     * Each frame applies forces or impulses which are scheduled by Push/Move
     */
    void Update() noexcept;
    
    /**
     *  Push the body with predefined impulses.
     * @param scale is a positive coefficient 
     *        which scaling the resulting impulse
     */
    void Push(Direction dir, float scale = 1.0f) noexcept;

    /**
     *  Move the body with predefined forces in one of 4 directions.
     * @param scale is a positive coefficient 
     *        which scaling the resulting velocity
     */
    void Move(Direction dir, float scale = 1.0f) noexcept;

    /**
     * Stops the movement in the given direction 
     * by reseting forces and velocity of the physic body
     */
    void Stop(Axis axis) noexcept;

    void SetMaxSpeed(float speed) noexcept;

    void SetJumpHeight(float height, float gravity) noexcept;

private:
    cocos2d::PhysicsBody * const m_body { nullptr };

    // jump speed calculated in runtime base on compile-time constants
    float m_upJumpSpeed { 0.f };
    // fall speed calculated in runtime base on compile-time constants
    float m_downJumpSpeed { 0.f };
    // max possible 
    float m_maxVelocity { 775.f };

    // set by user manually
    float m_desiredVelocity { 200.f };

    // The force scheduled to be applied to the body each update
    // can be reset by `Stop` function or overriden by the next `Move` call.
    // Used to move around like kinematic body and solve problem with 
    // body being `inserted into` and `pushed out of` another collidable body
    cocos2d::Vec2 m_force { 0.f, 0.f };

    // The impulse scheduled to be applied to the body NEXT update. 
    // Then it will be reseted to zero-vector
    cocos2d::Vec2 m_impulse { 0.f, 0.f };
};

#endif // MOVEMENT_HPP
