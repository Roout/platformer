#ifndef RESTART_SCENE_HPP
#define RESTART_SCENE_HPP

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class PauseNode : public cocos2d::Node {
public:
    static PauseNode* create(size_t id);

    bool init() override;

    ~PauseNode();
private:
    /**
     * @param id 
     *  Level's id you wish to restart.
     */
    PauseNode(size_t id);

    // id of the scene you with restart
    size_t m_id { 0 }; 
};

PauseNode* PauseNode::create(size_t id) {
    auto pRet = new (std::nothrow) PauseNode(id);
    if( pRet && pRet->init() ) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
}

bool PauseNode::init() {
    if(!cocos2d::Node::init()) {
        return false;
    }
    
    this->setName("Pause");

    auto restartButton = cocos2d::ui::Button::create("normal_restart.png", "selected_restart.png", "disabled_restart.png");
    restartButton->setTitleText("Restart");
    restartButton->addTouchEventListener([&](cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type) {
            switch (type) {
                case cocos2d::ui::Widget::TouchEventType::BEGAN:
                    break;
                case cocos2d::ui::Widget::TouchEventType::ENDED:
                    break;
                default:
                    break;
            }
    });
    this->addChild(restartButton);

    const auto resumeButton = cocos2d::ui::Button::create("normal_resume.png", "selected_resume.png", "disabled_resume.png");
    resumeButton->setTitleText("Resume");
    resumeButton->addTouchEventListener([&](cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type){
            switch (type) {
                case cocos2d::ui::Widget::TouchEventType::BEGAN: break;
                case cocos2d::ui::Widget::TouchEventType::ENDED: {
                    const auto scene = cocos2d::Director::getInstance()->getRunningScene();
                    const auto level = dynamic_cast<cocos2d::Scene*>(scene->getChildByName("Level"));
                    level->resume();
                    scene->getChildByName("Interface")->removeChildByName("Pause");
                } break;
                default: break;
            }
    });
    this->addChild(resumeButton);

    const auto size = this->getContentSize();

    restartButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    const cocos2d::Vec2 restartShift{ 0.f, restartButton->getContentSize().height / 3.f};
    restartButton->setPosition( restartShift + size / 2.f );

    resumeButton->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    const cocos2d::Vec2 resumeShift{ 0.f, -restartButton->getContentSize().height / 3.f};
    resumeButton->setPosition( resumeShift + size / 2.f );

    return true;
}


PauseNode::PauseNode(size_t id) {
}

PauseNode::~PauseNode()
{
}


#endif // RESTART_SCENE_HPP