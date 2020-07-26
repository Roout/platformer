#ifndef SMOOTH_FOLLOWER_HPP
#define SMOOTH_FOLLOWER_HPP 

#include "cocos2d.h"

class Unit;

class SmoothFollower final : public cocos2d::Vec2 {
public:
    SmoothFollower( Unit * const unit );

    /**
     * Smooth move from the current position to the new unit's position. 
     */
    void UpdateMapPosition(const float dt) ;

    /**
     * Called when Player::setPosition() is beeing called to update unit's start position! 
     */
    void Reset();
    
private:
    Unit * const m_unit { nullptr };

    cocos2d::Vec2 m_delta { 0.f, 0.f };
    //float m_speed { 0.f };
    /**
     * This is maximum velocity allowed for player follower.   
     */
    //static constexpr float m_maxSpeed { 400.f };
};

#endif // SMOOTH_FOLLOWER_HPP