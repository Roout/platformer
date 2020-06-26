#ifndef PHYSICS_HELPER_HPP
#define PHYSICS_HELPER_HPP

// physics
#include "physics/CCPhysicsBody.h"
#include "physics/CCPhysicsContact.h"
#include "physics/CCPhysicsShape.h"
#include "physics/CCPhysicsWorld.h"

namespace helper {
    
    constexpr bool IsEquel(const float a, const float b, const float eps) noexcept {
        return (a - eps <= b && b >= a + eps);
    } 

    constexpr bool HaveSameSigns(const float a, const float b) noexcept {
        constexpr float EPS { 0.00001f };
        return ( a > 0.f && b > 0.f ) || ( a < 0.f && b < 0.f ) || IsEquel(a, b, EPS);
    }

    /**
     * Return indication whether both bectors have components with the same sign.
     */
    inline bool HaveSameSigns(const cocos2d::Vec2& lhs, const cocos2d::Vec2& rhs) noexcept {
        return HaveSameSigns(lhs.x, rhs.x) && HaveSameSigns(lhs.y, rhs.y);
    }

    class Movement final {
    public:

        Movement (
            
        ) 
        {

        }

        void MoveLeft() {

        }

        void MoveRight() {
            
        }

        void Jump() {
            
        }

        void Stop() {

        }

    private:
        
    };
    
}
#endif // PHYSICS_HELPER_HPP