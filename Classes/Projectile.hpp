#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include "cocos2d.h"
#include <cstdint> // std::uint8_t
#include <string>
#include <initializer_list>

namespace dragonBones {
    class Animator;
}

class Projectile : public cocos2d::Node {
public:
    enum class State : std::uint8_t {
        IDLE = 0,
        EXPLODED,
        COUNT
    };
    
    static Projectile * create(float damage);

    [[nodiscard]] bool init() override;

    /**
     * This function update projectile state by keeping track 
     * of it's lifetime.
     */
    void update(float dt) override;

    void pause() override;
    
    void resume() override;

    void FlipX() noexcept; 

    /**
     * This function tells whether this prjectile still exist or not.
     * @return 
     *      The indication that the projectile still exist. 
     */
    [[nodiscard]] bool IsAlive() const noexcept {
        return m_currentState == State::IDLE;
    }

    /**
     * This method ends the projectile lifetime. So it will disappear. 
     */
    void Collapse() noexcept {
        m_lifeTime = 0.f;
    }

    [[nodiscard]] float GetDamage() const noexcept {
        return m_damage;
    }

    void SetLifetime(float seconds) noexcept {
        m_lifeTime = seconds;
    }

    void SetContactTestBitmask(size_t mask) noexcept;

    void SetCategoryBitmask(size_t mask) noexcept;

    /**
     * Create a dynamic body that is not influenced by gravity then add it to this node.
     * 
     * @return just created body
     */
    cocos2d::PhysicsBody* AddPhysicsBody(const cocos2d::Size& size);

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
     * Use when you're going to create an animated projectile 
     */
    void AddAnimator(std::string chachedArmatureName, std::string prefix = "");

    void InitializeAnimations(std::initializer_list<std::pair<std::size_t, std::string>> animations);

private:
    
    Projectile(float damage);

    void OnExplosion();

    void UpdateLifetime(const float dt) noexcept;

    void UpdateState(const float dt) noexcept;
    
    void UpdatePhysicsBody() noexcept;
    /**
     * Update animation according to the current state of Unit  
     */
    void UpdateAnimation();

    /**
     * Keep track of projectile lifetime. When 'm_lifeTime' <= 0.f
     * projectile should disappear.
     */
    float m_lifeTime { 0.f };

    const float m_damage { 0 };

    State m_previousState { State::IDLE };
    
    State m_currentState { State::IDLE };
    
    dragonBones::Animator * m_animator{ nullptr };

    cocos2d::Sprite * m_image { nullptr };

    // cocos2d::Size m_contentSize { 60.f, 135.f };
};
#endif // PROJECTILE_HPP