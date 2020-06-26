#include "ProjectileView.hpp"
#include "Weapon.hpp"
#include "PhysicsHelper.hpp"

ProjectileView * ProjectileView::create(const Projectile* const model) {
    auto pRet = new (std::nothrow) ProjectileView(model);
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool ProjectileView::init() {
    if( !cocos2d::DrawNode::init() ) {
        return false;
    }
    this->scheduleUpdate();
    // const auto body { m_model->GetBody() };
    // const auto size { m_model->GetSize() };
    // const auto position { m_model->GetBody()->getPosition() };
    // this->setPosition(position);
    
    //this->drawSolidRect(cocos2d::Vec2::ZERO, size, cocos2d::Color4F::BLUE );
    return true;
};

void ProjectileView::update([[maybe_unused]] float dt) {

}

ProjectileView::ProjectileView(const Projectile* model) :
    m_model { model }
{
}