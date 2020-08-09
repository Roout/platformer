#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Unit.hpp"
#include "cocos2d.h"
#include "Navigator.hpp"

namespace Enemies {

    class Warrior final: public Unit {
    public:
        static Warrior* create(const cocos2d::Size&, size_t id);

        [[nodiscard]] bool init() override;
        
        void update(float dt) override;

        inline size_t GetId() const noexcept;

        void AttachNavigator(
            const cocos2d::Size& mapSize, 
            float tileSize,
            path::Forest * const forest
        );

    private:
    
        Warrior(const cocos2d::Size&, size_t id);
        
        void UpdateState(const float dt) noexcept override;
    
    private:

        std::unique_ptr<Navigator> m_navigator { nullptr };

        const size_t m_id { 0 };
    };

    inline size_t Warrior::GetId() const noexcept {
        return m_id;
    }
};

#endif // ENEMY_HPP