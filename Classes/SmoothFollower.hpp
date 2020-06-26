#ifndef SMOOTH_FOLLOWER_HPP
#define SMOOTH_FOLLOWER_HPP 

#include "Unit.hpp"
#include "cocos2d.h"

class SmoothFollower final : public cocos2d::Vec2 {
public:
    SmoothFollower(std::shared_ptr<Unit> unit) :
        m_unit{ unit }
    {
        if( auto body = unit->GetBody(); body != nullptr ) {
            const auto position { body->getOwner()->getPosition()};
            // set up follower position
            x = position.x;
            y = position.y;
        }
    }

    /**
     * Smooth move from the current position to the new unit's position. 
     */
    void UpdateAfterUnitMove(const float dt) {
        if( const auto unit = m_unit.lock(); unit != nullptr ) {
            if( const auto body = unit->GetBody(); body != nullptr ) {
                const auto destination { body->getOwner()->getPosition() };
                static constexpr auto alpha { 0.1f }; 
                static constexpr auto eps { 0.1f }; 

                const auto pos = this->lerp(destination, alpha);
                if( pos.fuzzyEquals(destination, eps)) {
                    m_delta = destination - *this;
                    x = destination.x, y = destination.y;
                } else {
                    m_delta = pos - *this;
                    x = pos.x, y = pos.y;
                }
                
            }
        }
    }
    
    void UpdateNodePosition(cocos2d::Node *node) {
        const auto currentNodePos { node->getPosition()};
        node->setPosition(currentNodePos - m_delta);
    }

private:
    std::weak_ptr<Unit> m_unit;

    cocos2d::Vec2 m_delta { 0.f, 0.f };
    //float m_speed { 0.f };
    /**
     * This is maximum velocity allowed for player follower.   
     */
    //static constexpr float m_maxSpeed { 400.f };
};

#endif // SMOOTH_FOLLOWER_HPP