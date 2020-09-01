#ifndef UNIT_MOVEMENT_HPP
#define UNIT_MOVEMENT_HPP

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

    void Jump() noexcept;

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

    int m_remainingJumpSteps { 0 };

    bool m_isMovingLeft { false };

    bool m_isMovingRight { false };
};

#endif // UNIT_MOVEMENT_HPP
