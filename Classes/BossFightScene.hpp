#ifndef BOSS_FIGHT_SCENE_HPP
#define BOSS_FIGHT_SCENE_HPP

#include "LevelScene.hpp"

class BossFightScene final : public LevelScene {
public:
    
    [[nodiscard]] static cocos2d::Scene* createRootScene(int id);

    [[nodiscard]] static BossFightScene* create(int id);

    [[nodiscard]] bool init() override;

private:
    BossFightScene(int id);

};

#endif // BOSS_FIGHT_SCENE_HPP