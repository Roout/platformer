cmake_minimum_required(VERSION 3.8)

set(This dragon_bones)
set(TARGET_COCOS_VERSION 4.0)

project(${This} CXX)

include(animation/CMakeLists.txt)
include(armature/CMakeLists.txt)
include(cocos2dx/CMakeLists.txt)
include(core/CMakeLists.txt)
include(event/CMakeLists.txt)
include(factory/CMakeLists.txt)
include(geom/CMakeLists.txt)
include(model/CMakeLists.txt)
include(parser/CMakeLists.txt)


set(sources)
set(headers)

list(APPEND headers
	${DRAGONBONES_ANIMATION_HEADERS}
	${DRAGONBONES_ARMATURE_HEADERS}
	${DRAGONBONES_COCOS_HEADERS}
	${DRAGONBONES_CORE_HEADERS}
	${DRAGONBONES_EVENT_HEADERS}
	${DRAGONBONES_FACTORY_HEADERS}
	${DRAGONBONES_GEOM_HEADERS}
	${DRAGONBONES_MODEL_HEADERS}
	${DRAGONBONES_PARSER_HEADERS}
	DragonBonesHeaders.h
)

list(APPEND sources
	${DRAGONBONES_ANIMATION_SOURCES}
	${DRAGONBONES_ARMATURE_SOURCES}
	${DRAGONBONES_COCOS_SOURCES}
	${DRAGONBONES_CORE_SOURCES}
	${DRAGONBONES_EVENT_SOURCES}
	${DRAGONBONES_FACTORY_SOURCES}
	${DRAGONBONES_GEOM_SOURCES}
	${DRAGONBONES_MODEL_SOURCES}
	${DRAGONBONES_PARSER_SOURCES}	
)

# rapidjson library must be at the same folder as dragonBones
set(RAPIDJSON_DIR "../")

# debug message
# message(STATUS "Dragon Bones debug message: ")
# message(STATUS "\tHeaders: ${headers}")
# message(STATUS "\tSources: ${sources}")

# create static library
add_library(${This} STATIC ${sources} ${headers})

target_include_directories(${This} PRIVATE ${RAPIDJSON_DIR})

# debug message
message(STATUS "Dragon Bones debug message: ")
message(STATUS "\tLIBRARY_OUTPUT_DIRECTORY: ${CMAKE_BINARY_DIR}/lib")

set_target_properties(${This} 
	PROPERTIES 
	# PUBLIC_HEADER "DragonBonesHeaders.h"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    VERSION "${TARGET_COCOS_VERSION}"
    FOLDER "External"
	LINKER_LANGUAGE CXX
)

target_link_libraries(${This} PRIVATE cocos2d)