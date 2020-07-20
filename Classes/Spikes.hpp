#ifndef SPIKES_HPP
#define SPIKES_HPP

#include "cocos2d.h"
#include "Core.hpp"
#include "Utils.hpp"

/**
 * Do damage every m_maxCooldown seconds to each body it can influence
 */
class Spikes final {
public:
    Spikes( cocos2d::PhysicsBody * const body) 
        : m_body { body }
    {   
        m_body->setDynamic(false);
        m_body->setCategoryBitmask(
            Utils::CreateMask(
                core::CategoryBits::TRAP
            )
        );
        m_body->setContactTestBitmask(
            Utils::CreateMask(
                core::CategoryBits::ENEMY, 
                core::CategoryBits::HERO
            )
        );
    }


private:
    
    cocos2d::PhysicsBody * m_body { nullptr };

    static constexpr float m_damage { 10.f };
};

#endif // SPIKES_HPP