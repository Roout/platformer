#ifndef ENEMY_HPP
#define ENEMY_HPP

#include "Unit.hpp"
#include "cocos2d.h"

namespace Enemies {

    class Warrior final: public Unit {
    public:
        static Warrior* create(const cocos2d::Size&, size_t id);

        [[nodiscard]] bool init() override;

        // void update(float dt) override;

        size_t GetId() const noexcept {
            return m_id;
        }

    private:
    
        Warrior(const cocos2d::Size&, size_t id);
        
        void UpdateState(const float dt) noexcept override;
    
    private:

        const size_t m_id { 0 };
    };

};

#endif // ENEMY_HPP