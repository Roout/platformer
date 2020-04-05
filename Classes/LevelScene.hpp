#ifndef LEVEL_SCENE_HPP
#define LEVEL_SCENE_HPP

#include <memory>
#include "cocos2d.h"

class Unit;
class PhysicWorld;
class UserInputHandler;

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
// Order of creation: 
// * PhysicWorld => Unit => UserInputHandler
// Order of destruction
// * UserInputHandler => Unit => PhysicWorld
// It's obvious but I still would like to repeat this part here. 
    std::unique_ptr<PhysicWorld>        m_world { nullptr };
    std::unique_ptr<Unit>               m_unit { nullptr };
    std::unique_ptr<UserInputHandler>   m_inputHandler { nullptr };

    const int m_id { -1 }; 
};

#endif // LEVEL_SCENE_HPP