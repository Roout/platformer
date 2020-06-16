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
        UNDEFINED
    };
}

#endif // CORE_HPP