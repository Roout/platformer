#ifndef DASH_HPP__
#define DASH_HPP__

#include "cocos2d.h"
#include "EasyTimer.hpp"

#include "chipmunk/cpVect.h"

/** TODO:
 * - [x] inner cooldown like a weapon has 
 *       so that it will be impossible to move using only dashes
 * - [x] state accessor: is running or is stopped
 * - [x] restore previous state of the body when the dash stops
*/

class Dash : public cocos2d::Component {
public:
    static Dash* create(float cooldown, float initSpeed, float dashSpeed);
    
    void update(float dt) override;

    void Initiate(float dashDuration) noexcept;

    bool IsFinished() const noexcept;

    bool IsRunning() const noexcept;

    bool IsOnCooldown() const noexcept;

    void ResetSavedBodyState() noexcept;

private:

    Dash(float cooldown, float initSpeed, float dashSpeed)
        : m_cooldown { cooldown }
        , m_initialSpeed { initSpeed }
        , m_dashSpeed { dashSpeed }
    {}

    /// Properties
private:
    
    const float m_cooldown{ 0.f };
    float m_initialSpeed{ 0.f };
    float m_dashSpeed{ 0.f };

    EasyTimer m_dashTimer{}; 
    EasyTimer m_cooldownTimer{}; 

    /**
     * Stores physics body's state 
     */
    cpVect m_forces;
    cpVect m_velocity;
};

inline bool Dash::IsFinished() const noexcept {
    return m_dashTimer.IsFinished();
}
inline void Dash::ResetSavedBodyState() noexcept {
    m_forces = m_velocity = { 0.f, 0.f };
}

inline bool Dash::IsRunning() const noexcept {
    return !m_dashTimer.IsFinished();
}

inline bool Dash::IsOnCooldown() const noexcept {
    return (
        !m_cooldownTimer.IsFinished() 
        || !m_dashTimer.IsFinished()
    );
}

#endif // DASH_HPP__