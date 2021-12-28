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
    units/Warrior.hpp
    units/AxWarrior.hpp
    units/Wolf.hpp
    units/Wasp.hpp
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
    units/Warrior.cpp
    units/Wolf.cpp
    units/Wasp.cpp
    units/AxWarrior.cpp
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

# generate all required C++ header files from the JSON configuration files
set(json_schema_path "${CMAKE_SOURCE_DIR}/Resources/configuration")
set(generated_sources_path "${CMAKE_CURRENT_SOURCE_DIR}/configs")
# keep list of all json schemas 
set(json_schemas)
set(json_models)

# NOTE, schemas will be copied by cocos2d script 
# from outer CMakeLists.txt with another resources
list(APPEND json_schemas
    "units.json"
)

foreach(schema ${json_schemas})
    # generate header
    set(src)
    set(first_letter)
    # - remove suffix
    string(REPLACE ".json" "" src ${schema})
    # - capitalize the first letter
    string(SUBSTRING ${src} 0 1 first_letter)
    string(TOUPPER ${first_letter} first_letter)
    string(REGEX REPLACE "^.(.*)" "${first_letter}\\1" src "${src}")
    # - create a file
    string(CONCAT src "Json" ${src} ".hpp")

    add_custom_command(OUTPUT "${generated_sources_path}/${src}"
        COMMAND py ARGS ${CMAKE_CURRENT_SOURCE_DIR}/external/scripts/gen.py
            -i "${json_schema_path}/${schema}" -o "${generated_sources_path}/${src}"
        DEPENDS "${json_schema_path}/${schema}"
                "external/scripts/gen.py"
    )
    list(APPEND json_models "configs/${src}")
endforeach()

add_custom_target(libgen DEPENDS ${json_models})

# create static library
add_library(${This} STATIC ${sources} ${headers})
add_dependencies(${This} libgen)

target_compile_options(${This} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:Clang>:-Wall>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-Wall>>
    $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:MSVC>:/W3>>
)

# debug message
message(STATUS "\tLIBRARY_OUTPUT_DIRECTORY: ${CMAKE_BINARY_DIR}/lib")

# link dragonbones lib
set(DRAGONBONES_ROOT_PATH "external/dragonBones")
add_subdirectory(${DRAGONBONES_ROOT_PATH})

# remove rapidjson warnings
add_definitions(
	-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
	-D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
	-DRAPIDJSON_NOMEMBERITERATORCLASS
)

target_link_libraries(${This} PUBLIC cocos2d dragon_bones)

target_include_directories(${This}
    # to add a header from the dragon_bones library you need to specify `dragon_bones/...` manually
    PUBLIC ${DRAGONBONES_ROOT_PATH}/.. 
    PUBLIC ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
    PUBLIC units/
    PUBLIC configs/
)

set_target_properties(${This} 
    PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    FOLDER "External"
    LINKER_LANGUAGE CXX
)