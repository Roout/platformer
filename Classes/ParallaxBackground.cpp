#include "ParallaxBackground.hpp"

#include <cmath>

Background * Background::create() noexcept {
    auto pRet = new (std::nothrow) Background {};
    if(pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
} 

bool Background::init() {
    if(!cocos2d::Node::init()) {
        return false;
    }
    m_parallax = cocos2d::ParallaxNode::create();
    this->addChild(m_parallax);

    const float scale { 1.4f };
    const std::string m_filename { "Map/back.png" };

    auto layer = this->CreateLayer(m_filename, scale);
    if(layer) {
        m_parallax->addChild(layer, 0, cocos2d::Vec2(0.4f, 0.3f), {0.f, 0.f});
    }
    return true;
}

cocos2d::SpriteBatchNode * Background::CreateLayer(const std::string& filename, float scale) {
    auto director = cocos2d::Director::getInstance();
    auto textureCache = director->getTextureCache(); 
    if(auto texture = textureCache->addImage(filename); texture) {
        auto layer = cocos2d::SpriteBatchNode::createWithTexture(texture);
        auto width = floorf(texture->getContentSize().width * scale);
        for(int i = -1; i <= 1; i++) {
            auto node = cocos2d::Sprite::createWithTexture(texture);
            node->setScale(scale);
            node->setAnchorPoint({0.f, 0.f});
            node->setPosition({i * width, 0.f});
            layer->addChild(node);
        }
        return layer;
    }
    return nullptr;
}