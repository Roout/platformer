#ifndef DASH_HPP__
#define DASH_HPP__

#include "cocos2d.h"
#include "EasyTimer.hpp"

/** TODO:
 * - [x] inner cooldown like a weapon has 
 *       so that it will be impossible to move using only dashes
 * - [x] state accessor: is running or is stopped
 * - [x] stops body when it ends
*/

class Dash : public cocos2d::Component {
public:
    static Dash* create(float cooldown);
    
    void update(float dt) override;

    void Initiate(float dashDuration) noexcept;

    bool IsFinished() const noexcept;

    bool IsRunning() const noexcept;

    bool IsOnCooldown() const noexcept;

private:

    Dash(float cooldown)
        : m_cooldown { cooldown }
    {}

    /// Properties
private:
    
    const float m_cooldown { 0.f };

    EasyTimer m_dashTimer{}; 

    EasyTimer m_cooldownTimer{}; 

};

inline bool Dash::IsFinished() const noexcept {
    return m_dashTimer.IsFinished();
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