#ifndef BARREL_HPP
#define BARREL_HPP

#include "Core.hpp"
#include "cocos2d.h"
#include "SizeDeducer.hpp"

class Barrel final : public core::Entity {
public:
    Barrel() = default;

    ~Barrel() = default;
    
    void RecieveDamage( [[maybe_unused]] int damage) noexcept override {
        m_isExploded = true;
    } 

    bool IsExploded() const noexcept {
        return m_isExploded;
    }

    void AddPhysicsBody(cocos2d::PhysicsBody * const body) noexcept {
        m_body = body;
        m_body->setDynamic(false);
        m_body->setCategoryBitmask(
            Utils::CreateMask(
                core::CategoryBits::BARREL
            )
        );
        m_body->setCollisionBitmask(
            Utils::CreateMask(
                core::CategoryBits::BOUNDARY, 
                core::CategoryBits::PROJECTILE
            )
        );
    }

    const cocos2d::PhysicsBody * GetBody() const noexcept {
        return m_body;
    }

    /**
     * Return size of the barrel 
     * which was already adjusted to the resolution difference
     */
    const cocos2d::Size GetSize() const noexcept {
        return {
            SizeDeducer::GetInstance().GetAdjustedSize(m_width),
            SizeDeducer::GetInstance().GetAdjustedSize(m_height)
        };
    }

private:
    cocos2d::PhysicsBody * m_body { nullptr };

    bool m_isExploded { false };

private:

    static constexpr float m_width { 55.f };
    static constexpr float m_height { 120.f };
};

#endif // BARREL_HPP