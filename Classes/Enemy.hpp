#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <unordered_map>

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

        inline size_t GetId() const noexcept;

        void AttachNavigator(std::unique_ptr<Navigator> && navigator);

        void AttachInfluenceArea(const cocos2d::Rect& area);

        void Pursue(Unit * target) noexcept;

        void Patrol() noexcept;

        void OnEnemyIntrusion();

        void OnEnemyLeave();

    private:
        enum class State {
            UNDEFINED,
            
            PATROL,
            PURSUIT,
            ATTACK,
            DEATH,

            COUNT
        };

        std::string GetStateName(State state);

        Bot(size_t id);

        
        void UpdateState(const float dt) noexcept override;

        void UpdatePosition(const float dt) noexcept override;

        void UpdateAnimation() override;

        void UpdateDebugLabel() noexcept override;


        void AddPhysicsBody() override;

        void AddAnimator() override;

        void AddWeapon() override;


        void TryAttack();

        bool NeedAttack() const noexcept;

    private:

        std::unique_ptr<Navigator> m_navigator { nullptr };

        State m_currentState  { State::UNDEFINED };

        State m_previousState { State::UNDEFINED };

        bool m_detectEnemy { false };

        const size_t m_id { 0 };
    };

    inline size_t Bot::GetId() const noexcept {
        return m_id;
    }

}
#endif // ENEMY_HPP