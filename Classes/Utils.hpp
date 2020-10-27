#ifndef UNIQUE_UTILS_HPP
#define UNIQUE_UTILS_HPP

namespace Utils {
    /**
     * Work with enumerations: 
     */
    template<class Enum>
    constexpr size_t EnumCast(Enum value) noexcept {
        return static_cast<size_t>(value);
    }

    template<class Enum>
    constexpr size_t EnumSize() noexcept {
        return EnumCast(Enum::COUNT);
    } 

    template<class Enum>
    constexpr Enum EnumCast(size_t value) noexcept {
        return static_cast<Enum>(value);
    }

    template<class ...Args>
    constexpr size_t CreateMask(Args ... bits) noexcept {
        return (static_cast<size_t>(bits) | ...);
    } 

    template<class ...Args>
	constexpr bool HasAny(size_t mask, Args ... bits) noexcept {
		return (mask & Utils::CreateMask(bits...)) > 0;
	}

    /**
     * Generate numbers in range (0, 2^64].
     * Overflow is possible. (no check performed)
     */
    class LinearGenerator final {
    public:
        LinearGenerator(size_t value = 0U) : 
            m_value { value } {}

        size_t Next() const noexcept {
            return ++m_value;
        }

    private:
        mutable size_t m_value { 0 };
    };
}

#endif // UNIQUE_UTILS_HPP