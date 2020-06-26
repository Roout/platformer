#ifndef SPIKES_HPP
#define SPIKES_HPP

#include "cocos2d.h"
#include "Core.hpp"

/**
 * Do damage every m_maxCooldown seconds to each body it can influence
 */
class Spikes final : public core::Entity {
public:
    Spikes(
        const cocos2d::Vec2& pos, 
        const cocos2d::Size& size
    ) {   
        m_body = cocos2d::PhysicsBody::createBox(size);
        m_body->setDynamic(false);
        m_body->setPositionOffset(pos);
        m_body->setCategoryBitmask(
            core::CreateMask(
                core::CategoryBits::TRAP
            )
        );
        m_body->setCollisionBitmask(
            core::CreateMask(
                core::CategoryBits::ENEMY, 
                core::CategoryBits::HERO
            )
        );
    }

    ~Spikes() {
        m_body->removeFromWorld();
    }

    void Update(const float dt) {
        if(m_cooldown > 0.f) {
            m_cooldown -= dt;
        }
    }

    bool CanDamage() const noexcept {
        return m_cooldown <= 0.f;
    }

    float DealDamage() const noexcept {
        if( this->CanDamage() ) {
            m_cooldown = m_maxCooldown;
            return m_damage;
        } 
        else {
            return 0.f;
        }
    }

private:
    
    cocos2d::PhysicsBody * m_body { nullptr };

    float   m_cooldown { 0.f };

    static constexpr float m_maxCooldown { 0.3f };
    static constexpr float m_damage { 3.f };
};

#endif // SPIKES_HPP