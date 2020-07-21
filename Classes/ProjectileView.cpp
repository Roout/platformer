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

    return true;
};

void ProjectileView::update([[maybe_unused]] float dt) {

}

ProjectileView::ProjectileView(const Projectile* model) :
    m_model { model }
{
}