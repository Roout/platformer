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
        BARREL,
        PLAYER,
        ENEMY,
        SPIKES,
        INFLUENCE,
        
        COUNT
    };

    enum class EnemyClass {
        UNDEFINED,

        WARRIOR, // in fact it's axe-warrior
        ARCHER,
        SPEARMAN,

        COUNT
    };

    inline CategoryName CategoryFromString(const std::string& str) noexcept {
        CategoryName category { core::CategoryName::UNDEFINED };

        if( str == "platform" ) {
            category = CategoryName::PLATFORM;
        } else if(str == "border") {
            category = CategoryName::BORDER;
        } else if(str == "spikes") {
            category = CategoryName::SPIKES;
        } else if(str == "player") {
            category = CategoryName::PLAYER;
        } else if(str == "barrel") {
            category = CategoryName::BARREL;
        } else if(str == "enemy") {
            category = CategoryName::ENEMY;
        }

        return category;
    }

    enum class CategoryBits: int {
        HERO        = 0x0001,
        ENEMY       = 0x0002,
        BOUNDARY    = 0x0004,
        PROJECTILE  = 0x0008,
        PLATFORM    = 0x0010,
        TRAP        = 0x0020,
        BARREL      = 0x0040,
        GROUND_SENSOR = 0x0080
    };
}

#endif // CORE_HPP