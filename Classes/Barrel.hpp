#ifndef BARREL_HPP
#define BARREL_HPP

#include "cocos2d.h"

class Barrel final : public cocos2d::Node {
public:

    static Barrel * create();

    bool init() override;

    void update(float dt) override;

    void Explode();

private:
    
    Barrel();

     /**
     * Time which shows how long will the BarrelView exist 
     * since model was destroyed and @m_model pointer is dangling. 
     */
    float m_timeBeforeErasure { 0.485f };

    bool m_isExploded { false };

private:

    static constexpr float m_width { 55.f };
    static constexpr float m_height { 120.f };
};

#endif // BARREL_HPP