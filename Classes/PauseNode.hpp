#ifndef RESTART_SCENE_HPP
#define RESTART_SCENE_HPP

#include "cocos2d.h"

class PauseNode : public cocos2d::Node {
public:
    static PauseNode* create();

    bool init() override;
};

#endif // RESTART_SCENE_HPP