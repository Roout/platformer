#ifndef CORE_HPP
#define CORE_HPP

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
        UNDEFINED
    };

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

}

#endif // CORE_HPP