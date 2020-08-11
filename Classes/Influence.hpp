#ifndef AGRO_ZONE_HPP
#define AGRO_ZONE_HPP

#include "cocos2d.h"

namespace Enemies {
    class Warrior;
}

class Influence {
public:

    virtual ~Influence() = default;

    virtual void OnEnter() = 0;

    virtual void OnExit() = 0;

    virtual void Update() = 0;

    bool EnemyDetected() const noexcept {
        return m_detected;
    }

protected:
    cocos2d::Rect m_zone {};
    
    bool m_detected { false };
};

class WarriorInfluence final : public Influence {
public:

    inline void Attach(
        Enemies::Warrior * warrior, 
        const cocos2d::Rect& zone
    ) noexcept;

    void Update() override;

private:
    void OnEnter() override;

    void OnExit() override;

    Enemies::Warrior * m_warrior { nullptr };
};

inline void WarriorInfluence::Attach(
    Enemies::Warrior * warrior, 
    const cocos2d::Rect & zone
) noexcept {
    m_warrior = warrior;
    m_zone = zone;
}

#endif // AGRO_ZONE_HPP