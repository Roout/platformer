#ifndef CORE_HPP
#define CORE_HPP

#include <string>

namespace core {

    /**
     * This class declare interface which is used to effect derived entities 
     * on collision.
     */
    class Entity {
    public:

        virtual ~Entity() = default; 

        /**
         * This method functionality depends on implementation.
         * It either substract some health either do nothing.
         * Used in callbacks on collision.
         */
        virtual void RecieveDamage(int) noexcept {};
        
    };

    /**
     * This enumeration define different entity types,
     * including entities from the tile map. 
     * 
     * @note 
     *      It used as indexes for tilemap parser so don't change order.  
     */
    enum class CategoryName : int {
        PLATFORM,
        BORDER,
        BARREL,
        PLAYER,
        SPIKES,
        UNDEFINED,
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
        }

        return category;
    }

    /**
     * Underlying type of bitmask used to represent 
     * entity's category.
     */
    using MaskType = int;

    enum class CategoryBits: MaskType {
        HERO        = 0x0001,
        ENEMY       = 0x0002,
        BOUNDARY    = 0x0004,
        PROJECTILE  = 0x0008,
        PLATFORM    = 0x0010,
        TRAP        = 0x0020,
        BARREL      = 0x0040,
        HERO_SENSOR = 0x0080
    };

    template<class ...Args>
    constexpr MaskType CreateMask(Args ... bits) noexcept {
        return (static_cast<MaskType>(bits) | ...);
    } 

    template<class T>
    constexpr size_t EnumCast(T enumeration) noexcept {
        return static_cast<size_t> (enumeration);
    }

    template<class Enum>
    constexpr size_t EnumSize() noexcept {
        return static_cast<size_t> (Enum::COUNT);
    }

}

#endif // CORE_HPP