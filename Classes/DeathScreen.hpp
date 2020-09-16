#ifndef DEATH_SCREEN_HPP
#define DEATH_SCREEN_HPP

#include "cocos2d.h"

class DeathScreen : public cocos2d::Node {
public:
    static inline const char* const EVENT_NAME { "PLAYER_DEATH" };
    static inline const char* const NAME { "DEATH_SCREEN" };

    static DeathScreen* create();

    bool init() override;

    /// Lifetime management
    ~DeathScreen() = default;
private:

    DeathScreen() = default;
};

#endif // DEATH_SCREEN_HPP