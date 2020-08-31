#ifndef UNIT_MOVEMENT_HPP
#define UNIT_MOVEMENT_HPP

class Unit;

class Movement final {
public:
    Movement ( Unit * const unit ):
        m_unit { unit } 
    {}

    void Update(const float dt) noexcept;

    void Jump() noexcept;

    void MoveRight() noexcept;

    void MoveLeft() noexcept;

    void Stop() noexcept;
    
    void StopXAxisMove() noexcept;

    void SetMaxSpeed(float speed) noexcept;
    
private:
    Unit * const m_unit { nullptr };

    float m_desiredVelocity { 550.f };
    // static constexpr float  m_jumpHeight { 255.f };
    // static constexpr float  m_timeToJumpApex { 0.55 };

    static constexpr int m_timeStepsToCompletion { 6 };

    struct Counter {
        int remainingJumpSteps { 0 };
        int remainingMoveLeft { 0 };
        int remainingMoveRight { 0 };

        void Clear() {
            remainingJumpSteps = remainingMoveLeft = remainingMoveRight = 0;
        }

    } m_counter;
};

#endif // UNIT_MOVEMENT_HPP
