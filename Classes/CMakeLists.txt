cmake_minimum_required(VERSION 3.16)

set(This roout-classes)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)

set(headers)
set(sources)

list(APPEND headers
    scenes/Interface.hpp
    scenes/LevelScene.hpp
    scenes/BossFightScene.hpp
    scenes/PauseNode.hpp
    scenes/DebugScreen.hpp
    scenes/DeathScreen.hpp

    units/Unit.hpp
    units/Player.hpp
    units/Bot.hpp
    units/Wolf.hpp
    units/Wasp.hpp
    units/Warrior.hpp
    units/Slime.hpp
    units/Archer.hpp
    units/BoulderPusher.hpp
    units/Spider.hpp
    units/Spearman.hpp
    units/Cannon.hpp
    units/FireCloud.hpp
    units/BanditBoss.hpp
    units/Stalactite.hpp
    
    ContactHandler.hpp
    Influence.hpp
    Path.hpp
    Navigator.hpp
    Movement.hpp
    DragonBonesAnimator.hpp
    UserInputHandler.hpp
    TileMapParser.hpp
    TileMapHelper.hpp
    Props.hpp
    ParallaxBackground.hpp
    Weapon.hpp
    Core.hpp
    Utils.hpp
    Projectile.hpp
    Platform.hpp
    SizeDeducer.hpp
    SmoothFollower.hpp
    HealthBar.hpp
    Traps.hpp
    PhysicsHelper.hpp
    Curses.hpp
    CurseHub.hpp
    EasyTimer.hpp
    Dash.hpp
)

list(APPEND sources
    scenes/LevelScene.cpp
    scenes/BossFightScene.cpp
    scenes/PauseNode.cpp
    scenes/DebugScreen.cpp
    scenes/DeathScreen.cpp

    units/Unit.cpp
    units/Player.cpp
    units/Bot.cpp
    units/Wolf.cpp
    units/Wasp.cpp
    units/Warrior.cpp
    units/Slime.cpp
    units/Archer.cpp
    units/Cannon.cpp
    units/FireCloud.cpp
    units/BanditBoss.cpp
    units/Stalactite.cpp
    units/BoulderPusher.cpp
    units/Spider.cpp
    units/Spearman.cpp

    ContactHandler.cpp
    Influence.cpp
    Navigator.cpp
    DragonBonesAnimator.cpp
    Movement.cpp
    SmoothFollower.cpp
    UserInputHandler.cpp
    TileMapParser.cpp
    TileMapHelper.cpp
    Props.cpp
    ParallaxBackground.cpp
    Weapon.cpp
    Core.cpp
    HealthBar.cpp
    Curses.cpp
    CurseHub.cpp
    Projectile.cpp
    Dash.cpp
)

# create static library
add_library(${This} STATIC ${sources} ${headers})

target_compile_options(${This} PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:Clang>:-Wall>>
  $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-Wall>>
  $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/W3>>
)


# debug message
message(STATUS "Classes debug message: ")
message(STATUS "\tLIBRARY_OUTPUT_DIRECTORY: ${CMAKE_BINARY_DIR}/lib")

# link dragonbones lib
set(DRAGONBONES_ROOT_PATH "own-extension/dragonBones")
add_subdirectory(${DRAGONBONES_ROOT_PATH})

target_link_libraries(${This} PUBLIC cocos2d dragon_bones)

target_include_directories(${This}
    # to add a header from the dragon_bones library you need to specify `dragon_bones/...` manually
    PUBLIC ${DRAGONBONES_ROOT_PATH}/.. 
    PUBLIC ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
    PUBLIC units/
)

set_target_properties(${This} 
    PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    FOLDER "External"
    LINKER_LANGUAGE CXX
)