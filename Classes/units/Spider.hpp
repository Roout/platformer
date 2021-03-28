#ifndef SPIDER_HPP
#define SPIDER_HPP

#include "Bot.hpp"

#include <memory>

// Forward declarations:
class Navigator;
struct Path;

namespace Enemies {

/**
 * The spider only:
 * - moves up & down
 * - takes damage
 * - consist of move/die animations 
 */
class Spider final : public Bot {
public:
    static Spider* create(size_t id, const cocos2d::Size& contentSize);

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

    void onEnter() override;

    void onExit() override;

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

    void MoveAlong(float x, float y) noexcept override;

    /// Unique to Spider
    void AttachNavigator(Path&& path);

    void CreateWebAt(const cocos2d::Vec2& start);

private: 
    Spider(size_t id, 
        const char* dragonBonesName, 
        const cocos2d::Size& contentSize
    );

    void UpdateWeb();
    /// Bot interface
    
    void TryAttack();

    bool NeedAttack() const noexcept;
   
    void UpdateState(const float dt) noexcept override;

    void UpdatePosition(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;
    
    /// Properties
private:
    std::unique_ptr<Navigator> m_navigator { nullptr };

    cocos2d::Vec2 m_webStart {};
    cocos2d::DrawNode * m_web { nullptr };
};

} // namespace Enemies

#endif // SPIDER_HPP