#ifndef LEVEL_SCENE_HPP
#define LEVEL_SCENE_HPP

#include <memory>
#include "cocos2d.h"
#include "../TileMapParser.hpp"

class LevelScene : public cocos2d::Scene {
public:

/// Constants which define jump height and time for PLAYER!
/// NOTE! GRAVITY is fully based on player!
    // Defines how high can the body jump
    static constexpr float JUMP_HEIGHT { 130.f };     // up to 3 tiles
    // Defines how fast the body reach the max height by single jump 
    static constexpr float TIME_OF_APEX_JUMP { 0.3f };  // standart time
    // Define gravity for this level
    static constexpr float GRAVITY {  // Calculate gravity base on defined constancts: height, time ( G = -H / (2*t*t) )
        -JUMP_HEIGHT / (2 * TIME_OF_APEX_JUMP * TIME_OF_APEX_JUMP) 
    };

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