cmake_minimum_required(VERSION 3.16)

set(This roout-classes)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)

set(headers)
set(sources)

list(APPEND headers
    LevelScene.hpp
    ContactHandler.hpp
    PauseNode.hpp
    DebugScreen.hpp
    Unit.hpp
    DeathScreen.hpp
    Player.hpp
    Bot.hpp
    Warrior.hpp
    Archer.hpp
    BoulderPusher.hpp
    Spider.hpp
    Spearman.hpp
    Influence.hpp
    Path.hpp
    Navigator.hpp
    Movement.hpp
    DragonBonesAnimator.hpp
    UserInputHandler.hpp
    TileMapParser.hpp
    TileMapHelper.hpp
    ParallaxBackground.hpp
    Weapon.hpp
    Core.hpp
    Utils.hpp
    Projectile.hpp
    Platform.hpp
    Barrel.hpp
    SizeDeducer.hpp
    SmoothFollower.hpp
    HealthBar.hpp
    Interface.hpp
    Traps.hpp
    PhysicsHelper.hpp
    Curses.hpp
    CurseHub.hpp
)

list(APPEND sources
    LevelScene.cpp
    ContactHandler.cpp
    Unit.cpp
    Player.cpp
    Influence.cpp
    Navigator.cpp
    Bot.cpp
    Warrior.cpp
    Archer.cpp
    BoulderPusher.cpp
    Spider.cpp
    Spearman.cpp
    PauseNode.cpp
    DebugScreen.cpp
    DragonBonesAnimator.cpp
    DeathScreen.cpp
    Movement.cpp
    SmoothFollower.cpp
    UserInputHandler.cpp
    TileMapParser.cpp
    TileMapHelper.cpp
    ParallaxBackground.cpp
    Weapon.cpp
    HealthBar.cpp
    Curses.cpp
    CurseHub.cpp
    Barrel.cpp
    Projectile.cpp
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
)

set_target_properties(${This} 
    PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    FOLDER "External"
    LINKER_LANGUAGE CXX
)