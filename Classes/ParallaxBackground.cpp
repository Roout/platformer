#include "ParallaxBackground.hpp"

#include <array>
#include <cmath>
#include <string>

Background * Background::create(const cocos2d::Size& size ) noexcept {
    auto pRet = new (std::nothrow) Background {size};
    if(pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
} 

Background::Background(const cocos2d::Size& size) 
    : m_mapSize { size }
{}

bool Background::init() {
    if(!cocos2d::Node::init()) {
        return false;
    }
    m_parallax = cocos2d::ParallaxNode::create();
    this->addChild(m_parallax);

    struct ParallaxLayer {
        float scale;
        std::string path;
        int zOrder;
        cocos2d::Vec2 ratio;
        cocos2d::Vec2 offset;
    };

    const std::array<ParallaxLayer, 5U> layers = {
          ParallaxLayer{ 1.f, "Map/back/sky.png",   0, {0.f, 0.f},      {0.f, 600.f} }
        , ParallaxLayer{ 1.f, "Map/back/sky-0.png", 1, {0.1f, 0.15f},   {0.f, -60.f} }  // clouds
        , ParallaxLayer{ 1.f, "Map/back/2.png",     2, {0.4f, 0.1f},    {0.f, 0.f} }    // mountains
        , ParallaxLayer{ 1.f, "Map/back/sky-1.png", 3, {0.25f, 0.1f},   {0.f, -80.f} }  // cloud
        , ParallaxLayer{ 1.f, "Map/back/1.png",     4, {0.3f, 0.2f},    {0.f, -100.f} } // trees
    };

    for(auto& layer: layers) {
        if(auto back = this->CreateLayer(layer.path, layer.scale)) {
            m_parallax->addChild(back, layer.zOrder, layer.ratio, layer.offset);
        }
    }
    
    return true;
}

cocos2d::SpriteBatchNode * Background::CreateLayer(const std::string& filename, float scale) {
    auto director = cocos2d::Director::getInstance();
    auto textureCache = director->getTextureCache(); 
    if(auto texture = textureCache->addImage(filename); texture) {
        auto layer = cocos2d::SpriteBatchNode::createWithTexture(texture);
        auto textureWidth = floorf(texture->getContentSize().width * scale);
        // Horizontal repeat
        // Lets start with shift equals to half width of the texture
        auto xStart = -floorf(textureWidth / 2.f);
        auto xFinish = m_mapSize.width - floorf(textureWidth / 2.f);
        auto xPosition = xStart;
        for(int i = 0; xPosition <= xFinish; i++) {
            auto node = cocos2d::Sprite::createWithTexture(texture);
            node->setScale(scale);
            node->setAnchorPoint({0.f, 0.f});
            xPosition = xStart + static_cast<float>(i * textureWidth);
            node->setPosition({xPosition, 0.f});
            layer->addChild(node);
        }
        return layer;
    }
    return nullptr;
}