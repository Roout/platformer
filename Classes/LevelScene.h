#ifndef LEVEL_SCENE_H
#define LEVEL_SCENE_H

#include "cocos2d.h"


class LevelScene final : public cocos2d::Scene {
public:

    [[nodiscard]] static LevelScene* create(int id);

    [[nodiscard]] bool init() override;
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);

	void update(float dt) override;

	~LevelScene() = default;

    LevelScene(const LevelScene&) = delete;

    LevelScene& operator=(const LevelScene&) = delete;

private:
	LevelScene(int id);

    const int m_id { -1 }; 
};

#endif