#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include "cocos2d.h"

class Projectile : public cocos2d::Node {
public:
    static Projectile * create(float damage);

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

    void SetLifetime(float seconds) noexcept {
        m_lifeTime = seconds;
    }

    void SetContactTestBitmask(size_t mask) noexcept;

    void SetCategoryBitmask(size_t mask) noexcept;

    /**
     * Create a sprite for the projectile
     * 
     * @param imagePath specify the path to image
     * 
     * @note it may needs to be flipped depends on velocity vector  
     * @code
     * if(velocity.x > 0.f) {
     *    image->setFlippedX(true);
     * }
     * @endcode
     */
    cocos2d::Sprite* AddImage(const char* imagePath);

    /**
     * Create a dynamic body that is not influenced by grabity then add it to this node.
     * 
     * @return just created body
     */
    cocos2d::PhysicsBody* AddPhysicsBody(const cocos2d::Size& size);

private:
    
    Projectile(float damage);

    /**
     * Keep track of projectile lifetime. When 'm_lifeTime' <= 0.f
     * projectile should disappear.
     */
    float m_lifeTime { 0.f };

    const float m_damage { 0 };
};

#endif // PROJECTILE_HPP