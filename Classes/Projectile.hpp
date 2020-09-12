#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include "cocos2d.h"

class Projectile : public cocos2d::Node {
public:
    static Projectile * create(
        const cocos2d::Size& size,
        const cocos2d::Vec2& velocity,
        const float damage
    );

    bool init() override;

    /**
     * This function update projectile state by keeping track 
     * of it's lifetime.
     */
    void update(float dt) override;

    /**
     * This function tells whether this prjectile still exist or not.
     * @return 
     *      The indication that the projectile still exist. 
     */
    [[nodiscard]] bool IsAlive() const noexcept {
        return m_lifeTime > 0.f;
    }

    /**
     * This method ends the projectile lifetime. So it will disappear. 
     */
    void Collapse() noexcept {
        m_lifeTime = 0.f;
    }

    float GetDamage() const noexcept {
        return m_damage;
    }

    void SetLifetime(float time) noexcept {
        m_lifeTime = time;
    }

    void SetContactTestBitmask(size_t mask) noexcept;

    void AddImage(const char* imagePath);

private:
    
    Projectile(
        const cocos2d::Size& size,
        const cocos2d::Vec2& velocity,
        const float damage
    );
    /**
     * Keep track of projectile lifetime. When 'm_lifeTime' <= 0.f
     * projectile should disappear.
     */
    float m_lifeTime { 0.f };

    const float m_damage { 0 };
};

#endif // PROJECTILE_HPP