#ifndef UNIT_MOVEMENT_HPP
#define UNIT_MOVEMENT_HPP

#include <array>
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
    Movement(cocos2d::PhysicsBody * const body);
    
    ~Movement();

    void Update(const float dt) noexcept;

    void MoveUp() noexcept;

    void MoveDown() noexcept;

    void MoveRight() noexcept;

    void MoveLeft() noexcept;

    void Stop() noexcept;
    
    void SetMaxSpeed(float speed) noexcept;

private:
    cocos2d::PhysicsBody * const m_body { nullptr };

    float m_desiredVelocity { 550.f };
    // static constexpr float  m_jumpHeight { 255.f };
    // static constexpr float  m_timeToJumpApex { 0.55 };

    static constexpr int m_timeStepsToCompletion { 6 };

    int m_remainingAirSteps { 0 };

    enum class Direction { LEFT, RIGHT, UP, DOWN, COUNT };
    
    std::array<bool, Utils::EnumSize<Direction>()> m_indicators;

};

#endif // UNIT_MOVEMENT_HPP
