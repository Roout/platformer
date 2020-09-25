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
    static Spider* create(size_t id);

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

    /// Unique to warrior
    void AttachNavigator(Path&& path);

private: 
    Spider(size_t id, const char* dragonBonesName);
    
    void TryAttack();

    bool NeedAttack() const noexcept;

    /// Bot interface
   
    void UpdateState(const float dt) noexcept override;

    void UpdatePosition(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;
    
    /// Properties
private:
    std::unique_ptr<Navigator> m_navigator { nullptr };
};

} // namespace Enemies

#endif // SPIDER_HPP