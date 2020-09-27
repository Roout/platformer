#ifndef RESTART_SCENE_HPP
#define RESTART_SCENE_HPP

#include "cocos2d.h"

class PauseNode : public cocos2d::Node {
public:
    static PauseNode* create();

    bool init() override;

    void onEnter() override;
    
    void onExit() override;
};

#endif // RESTART_SCENE_HPP