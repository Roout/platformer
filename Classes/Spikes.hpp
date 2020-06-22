#ifndef SPIKES_HPP
#define SPIKES_HPP

#include "PhysicWorld.hpp"
#include "Core.hpp"

/**
 * Do damage every m_maxCooldown seconds to each body it can influence
 */
class Spikes final : public core::Entity {
public:
    Spikes(PhysicWorld * const world, float x, float y, float w, float h) :
        m_world { world }
    {   
        const auto callback = [x,y, this](core::Entity * entity ) {
            cocos2d::log("Spikes at [%f, %f] collide with some entity!\n It deal %f damage!",
                x / 80.f, 
                y / 80.f, 
                m_damage 
            );
            if( entity != nullptr ) {
                entity->RecieveDamage(static_cast<int>(this->DealDamage()));
            }
        };
        m_body = m_world->Create<StaticBody>(
            callback,
            cocos2d::Vec2{ x, y }, cocos2d::Size{ w, h } 
        );
        m_body->EmplaceFixture(this, core::CategoryName::SPIKES);
        m_body->SetMask(
            CreateMask(CategoryBits::TRAP),
            CreateMask(CategoryBits::ENEMY, CategoryBits::HERO) 
        );
    }

    ~Spikes() {
        m_world->Erase(m_body);
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
    PhysicWorld * const m_world { nullptr };
    
    StaticBody * m_body { nullptr };

    float   m_cooldown { 0.f };

    static constexpr float m_maxCooldown { 0.3f };
    static constexpr float m_damage { 3.f };
};

#endif // SPIKES_HPP