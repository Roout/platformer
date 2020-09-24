#ifndef INFLUENCE_ZONE_HPP
#define INFLUENCE_ZONE_HPP

#include "cocos2d.h"

namespace Enemies {
    class Bot;
}

class Influence : public cocos2d::Component {
public:
    static Influence* create(
        Enemies::Bot* bot, 
        const cocos2d::Rect& zone
    );
    
    void update(float [[maybe_unused]] dt) override;

    bool Contains(const cocos2d::Vec2& point) const noexcept;

private:

    Influence(
        Enemies::Bot* bot, 
        const cocos2d::Rect& zone
    );

    // target intrude into the influence zone
    void OnIntrusion();

    // target leave the influence zone
    void OnLeave();

    /// Properties
private:
    cocos2d::Rect m_zone {};
    
    bool m_detected { false };

    Enemies::Bot * m_bot { nullptr };
};

#endif // INFLUENCE_ZONE_HPP