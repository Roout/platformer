#ifndef PLATFORMER_DEBUG_SCREEN_HPP
#define PLATFORMER_DEBUG_SCREEN_HPP

#include "cocos2d.h"

namespace screen {

/**
 * - [x] pause/resume playing scene (level)
 * - [x] switch physics world debug mode
 * - [x] switch GOD mode
 */
class DebugScreen : public cocos2d::Node {
public:
    inline static const char * const EVENT_NAME = "DEBUG_EVENT";
    inline static const char * const NAME = "DEBUG_SCREEN";

    static DebugScreen* create();

    bool init() override;

    // pause level scene on enter
    void onEnter() override;

    // resume level scene on exit
    void onExit() override;

    void SwitchPhysicsDebugMode() noexcept;

};

} // namespace screen

#endif // PLATFORMER_DEBUG_SCREEN_HPP