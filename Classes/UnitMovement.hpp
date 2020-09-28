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
    static constexpr float m_jumpHeight { 255.f };     // up to 3 tiles
    static constexpr float m_timeToJumpApex { 0.30 };  // standart time
    static constexpr float m_gravity { 
        -m_jumpHeight / (2 * m_timeToJumpApex * m_timeToJumpApex) 
    };
    
    Movement(cocos2d::PhysicsBody * const body);
    
    ~Movement();

    void Update(const float dt) noexcept;

    void Jump() noexcept;

    void MoveUp() noexcept;

    void MoveDown() noexcept;

    void MoveRight() noexcept;

    void MoveLeft() noexcept;

    void Stop() noexcept;
    
    void SetMaxSpeed(float speed) noexcept;

private:
    cocos2d::PhysicsBody * const m_body { nullptr };

    const float m_maxVelocity { 1550.f };
    static constexpr int    m_timeStepsToCompletion { 1 };

    float m_desiredVelocity { 550.f };

    int m_remainingAirSteps { 0 };
    
    enum class Action { 
        JUMP, 
        MOVE_LEFT, 
        MOVE_RIGHT, 
        MOVE_UP, 
        MOVE_DOWN, 
        COUNT 
    };

    std::array<bool, Utils::EnumSize<Action>()> m_indicators;
};

#endif // UNIT_MOVEMENT_HPP
