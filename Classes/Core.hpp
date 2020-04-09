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

        // virtual void RecieveDamage(int) noexcept = 0;
    };
}

#endif // CORE_HPP