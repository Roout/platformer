#ifndef DRAWABLE_H
#define DRAWABLE_H
#include "cocos2d.h"

namespace drawable 
{
    /*
        @return drawable autorelease cocos2d::DrawNode
    */
    [[nodiscard]] inline cocos2d::DrawNode * CreateRectNode(
            const cocos2d::Rect &rect, 
            const cocos2d::Color4F &fillColor = { 1.f, 1.f, 1.f, 1.f },
            const float boarderWidth = 1.f,
            const cocos2d::Color4F &boarderColor =  { 1.f, 1.f, 1.f, 1.f }
    ) {
        auto rectNode = cocos2d::DrawNode::create();
        static constexpr auto pointCount { 4 };
        const cocos2d::Vec2 rectangle[pointCount] = {
            { rect.origin.x, rect.origin.y },
            { rect.origin.x, rect.origin.y + rect.size.height },
            { rect.origin.x + rect.size.width, rect.origin.y + rect.size.height },
            { rect.origin.x + rect.size.width, rect.origin.y },
        };
        rectNode->drawPolygon(rectangle, pointCount, fillColor, boarderWidth, boarderColor);
        return rectNode;
    }
}


#endif // DRAWABLE_H