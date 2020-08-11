#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Unit.hpp"
#include "cocos2d.h"
#include "Navigator.hpp"
#include "Influence.hpp"

namespace Enemies {

    class Warrior final: public Unit {
    public:
        static constexpr char * const NAME = "Warrior";

        static Warrior* create(const cocos2d::Size&, size_t id);

        [[nodiscard]] bool init() override;
        
        void update(float dt) override;

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

    private:
    
        Warrior(const cocos2d::Size&, size_t id);
        
        void UpdateState(const float dt) noexcept override;

        bool NeedAttack() const noexcept;

    private:

        std::unique_ptr<Navigator> m_navigator { nullptr };

        WarriorInfluence m_influence;

        const size_t m_id { 0 };
    };

    inline size_t Warrior::GetId() const noexcept {
        return m_id;
    }
}

#endif // ENEMY_HPP