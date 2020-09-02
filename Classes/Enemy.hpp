#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Unit.hpp"
#include "cocos2d.h"
#include "Navigator.hpp"
#include "Influence.hpp"

namespace Enemies {

    class Bot final: public Unit {
    public:
        static constexpr char * const NAME = "Warrior";

        static Bot* create(size_t id);

        [[nodiscard]] bool init() override;
        
        void update(float dt) override;

        void pause() override;

        void resume() override;

        inline size_t GetId() const noexcept;

        void AttachNavigator(
            const cocos2d::Size& mapSize, 
            float tileSize,
            path::Supplement * const
        );

        void AttachInfluenceArea(
            const cocos2d::Size& mapSize, 
            float tileSize,
            path::Supplement * const
        );

        void Pursue(Unit * target) noexcept;

        void Patrol() noexcept;

        void OnEnemyIntrusion();

        void OnEnemyLeave();

    private:
    
        Bot(size_t id);
        
        void UpdateState(const float dt) noexcept override;

        void UpdateAnimation() override;

        void AddPhysicsBody(const cocos2d::Size&) override;

        void AddAnimator() override;

        void UpdatePosition(const float dt) noexcept override;

        void TryAttack();

        bool NeedAttack() const noexcept;

    private:

        std::unique_ptr<Navigator> m_navigator { nullptr };

        bool m_detectEnemy { false };

        const size_t m_id { 0 };
    };

    inline size_t Bot::GetId() const noexcept {
        return m_id;
    }
}

#endif // ENEMY_HPP