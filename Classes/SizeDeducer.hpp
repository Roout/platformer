#ifndef SIZE_DEDUCER_HPP
#define SIZE_DEDUCER_HPP

#include "cocos2d.h"

class SizeDeducer final {
public:
/// Life cycle management
    SizeDeducer(const SizeDeducer&) = delete;
    SizeDeducer& operator=(const SizeDeducer&) = delete;

    SizeDeducer(SizeDeducer&&) = delete;
    SizeDeducer& operator=(SizeDeducer&&) = delete;

public:
    static SizeDeducer& GetInstance() {
        static SizeDeducer deducer {};
        return deducer;
    } 

    /// TODO: add type checks
    template<class T>
    T GetAdjustedSize(const T value) const noexcept {
        return static_cast<T>(value * m_scaleFactor);
    } 

    float GetScaleFactor() const noexcept {
        return m_scaleFactor;
    }

private:
    SizeDeducer() :
        m_scaleFactor { cocos2d::Director::getInstance()->getContentScaleFactor() }
    {
    }

    const float m_scaleFactor { 1.f };
};

#endif // SIZE_DEDUCER_HPP