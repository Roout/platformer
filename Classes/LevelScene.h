#ifndef LEVEL_SCENE_H
#define LEVEL_SCENE_H

#include "cocos2d.h"

class LevelScene final : public cocos2d::Scene {
public:
    [[nodiscard]] static cocos2d::Scene* createScene();

    [[nodiscard]] static LevelScene* create();

    [[nodiscard]] bool init() override;
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);

	void update(float dt) override;

	~LevelScene() = default;

    LevelScene(const LevelScene&) = delete;

    LevelScene& operator=(const LevelScene&) = delete;

private:
	LevelScene() = default;
};

#endif