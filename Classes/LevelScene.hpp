#ifndef LEVEL_SCENE_HPP
#define LEVEL_SCENE_HPP

#include <memory>
#include "cocos2d.h"
#include "TileMapParser.hpp"

class LevelScene : public cocos2d::Scene {
public:

    [[nodiscard]] static cocos2d::Scene* createRootScene(int id);

    [[nodiscard]] static LevelScene* create(int id);

    [[nodiscard]] bool init() override;
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);

    void pause() override;

    void resume() override;

    void onEnter() override;

    void onExit() override;

    void Restart();

    /// Lifecycle
	~LevelScene() = default;

    LevelScene(const LevelScene&) = delete;

    LevelScene& operator=(const LevelScene&) = delete;

protected:
	LevelScene(int id);

    virtual void InitTileMapObjects(cocos2d::FastTMXTiledMap * map);

    std::unique_ptr<TileMapParser> m_parser { nullptr };

    // level id. Used to load a map
    const int m_id { -1 }; 

    std::string m_tmxFile;
};

#endif // LEVEL_SCENE_HPP