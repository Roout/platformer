#ifndef LEVEL_SCENE_HPP
#define LEVEL_SCENE_HPP

#include <memory>
#include "cocos2d.h"

class Unit;
class Movement;
class UserInputHandler;
class SmoothFollower;

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

    void InitTileMapObjects(cocos2d::FastTMXTiledMap * map);

    // model:
    std::shared_ptr<Unit>               m_unit { nullptr };
    cocos2d::Vec2                       m_playerPosition { 0.f, 0.f };
    // controller:
    std::unique_ptr<SmoothFollower>     m_playerFollower { nullptr };
    std::unique_ptr<Movement>           m_movement { nullptr };
    std::unique_ptr<UserInputHandler>   m_inputHandler { nullptr };

    // level id. Used to load a map
    const int m_id { -1 }; 
};

#endif // LEVEL_SCENE_HPP