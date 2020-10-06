#ifndef PARALLAX_BACKGROUND_HPP
#define PARALLAX_BACKGROUND_HPP

#include "cocos2d.h"

#include <string>

class Background : public cocos2d::Node {
public:
    static Background * create() noexcept;

    [[nodiscard]] bool init() override;

private:

    cocos2d::SpriteBatchNode * CreateLayer(const std::string& filename, float scale);

    // no ownerships just view
    cocos2d::ParallaxNode * m_parallax { nullptr };
};


#endif // PARALLAX_BACKGROUND_HPP