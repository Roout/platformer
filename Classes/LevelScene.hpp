#ifndef LEVEL_SCENE_HPP
#define LEVEL_SCENE_HPP

#include <memory>
#include <vector>
#include "Border.hpp"
#include "Platform.hpp"
#include "cocos2d.h"

class Unit;
class Movement;
class UserInputHandler;
class BarrelManager;
class SmoothFollower;

class LevelScene final : public cocos2d::Scene {
public:

    [[nodiscard]] static cocos2d::Scene* createScene(int id);

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

    // model:
    std::shared_ptr<Unit>               m_unit { nullptr };
    // controller:
    std::unique_ptr<SmoothFollower>     m_playerFollower { nullptr };
    std::unique_ptr<Movement>           m_movement { nullptr };
    std::unique_ptr<UserInputHandler>   m_inputHandler { nullptr };

    // models
    std::vector<Border>     m_borders;
    std::vector<Platform>   m_platforms;
    // controller
    std::unique_ptr<BarrelManager>      m_barrelManager { nullptr };

    // level id. Used to load a map
    const int m_id { -1 }; 
};

#endif // LEVEL_SCENE_HPP