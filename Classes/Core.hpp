#ifndef CORE_HPP
#define CORE_HPP

#include <string>

namespace core {
    /**
     * This enumeration define different entity types,
     * including entities from the tile map. 
     * 
     * @note 
     *      It used as indexes for tilemap parser so don't change order.  
     */
    enum class CategoryName {
        UNDEFINED,
        
        PLATFORM,
        BORDER,
        PROPS,
        PLAYER,
        ENEMY,
        SPIKES,
        INFLUENCE,
        PATH,
        
        COUNT
    };

    enum class EnemyClass {
        UNDEFINED,

        WARRIOR, // in fact it's axe-warrior
        ARCHER,
        SLIME,
        BOULDER_PUSHER,
        SPEARMAN,
        SPIDER,

        COUNT
    };

    namespace EntityNames {
        const char* const WARRIOR   = "warrior";
        const char* const ARCHER    = "archer";
        const char* const BOULDER_PUSHER = "old_man";
        const char* const SPIDER    = "spider";
        const char* const SLIME    = "slime";
        const char* const SPEARMAN  = "spear_man";
        const char* const PLAYER    = "player";
    }

    inline CategoryName CategoryFromString(const std::string& str) noexcept {
        CategoryName category { core::CategoryName::UNDEFINED };

        if(str == "platform" ) {
            category = CategoryName::PLATFORM;
        } 
        else if(str == "border") {
            category = CategoryName::BORDER;
        } 
        else if(str == "spikes") {
            category = CategoryName::SPIKES;
        } 
        else if(str == "player") {
            category = CategoryName::PLAYER;
        } 
        else if(str == "props") {
            category = CategoryName::PROPS;
        } 
        else if(str == "enemy") {
            category = CategoryName::ENEMY;
        }
        else if(str == "path") {
            category = CategoryName::PATH;
        }
        else if(str == "influence") {
            category = CategoryName::INFLUENCE;
        }

        return category;
    }

    enum class CategoryBits: int {
        PLAYER              = 0x0001,
        ENEMY               = 0x0002,
        BOUNDARY            = 0x0004,
        ENEMY_PROJECTILE    = 0x0008,
        PLAYER_PROJECTILE   = 0x0010,
        PLATFORM            = 0x0020,
        TRAP                = 0x0040,
        PROPS               = 0x0080,
        GROUND_SENSOR       = 0x0100,
        HITBOX_SENSOR       = 0x0200
    };
}

#endif // CORE_HPP