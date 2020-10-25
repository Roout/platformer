#ifndef PHYSICS_HELPER_HPP
#define PHYSICS_HELPER_HPP

namespace helper {
    
    constexpr bool IsEqual(const float a, const float b, const float eps) noexcept {
        return (a - eps <= b && b <= a + eps);
    } 

    constexpr bool IsPositive(const float x, const float eps) noexcept {
        return !IsEqual(x, 0.f, eps) && x > 0.f;
    }

    constexpr bool IsNegative(const float x, const float eps) noexcept {
        return !IsEqual(x, 0.f, eps) && x < 0.f;
    }

    constexpr bool IsGreater(const float a, const float b, const float eps) noexcept {
        return !IsEqual(a, b, eps) && a > b;
    }

    constexpr bool IsLesser(const float a, const float b, const float eps) noexcept {
        return !IsEqual(a, b, eps) && a < b;
    }


    constexpr bool HaveSameSigns(const float a, const float b) noexcept {
        constexpr float EPS { 0.00001f };
        return ( a > 0.f && b > 0.f ) || ( a < 0.f && b < 0.f ) || IsEqual(a, b, EPS);
    }

    /**
     * Return indication whether both vectors have components with the same sign.
     */
    inline bool HaveSameSigns(const cocos2d::Vec2& lhs, const cocos2d::Vec2& rhs) noexcept {
        return HaveSameSigns(lhs.x, rhs.x) && HaveSameSigns(lhs.y, rhs.y);
    }
}
#endif // PHYSICS_HELPER_HPP