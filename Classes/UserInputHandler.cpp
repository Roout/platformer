#include "UserInputHandler.h"
#include "Unit.h"
#include "PhysicWorld.h" // KinematicBody access

#include "cocos2d.h"

UserInputHandler::Input::Input(WinKeyCode keyCode) {
    if (keyCode == WinKeyCode::KEY_LEFT_ARROW ||
        keyCode == WinKeyCode::KEY_A) 
    {
        dx = -1;
    }
    else if (keyCode == WinKeyCode::KEY_RIGHT_ARROW ||
            keyCode == WinKeyCode::KEY_D)
    {
        dx = 1;
    }
    else if (keyCode == WinKeyCode::KEY_UP_ARROW ||
            keyCode == WinKeyCode::KEY_W || 
            keyCode == WinKeyCode::KEY_SPACE)
    {
        jump = true;
    }
}

UserInputHandler::UserInputHandler(Unit * const model, cocos2d::Node * const node ):
    m_model { model }
{
    auto listener = cocos2d::EventListenerKeyboard::create();
    
    listener->onKeyPressed = [this](WinKeyCode keyCode, cocos2d::Event* event) {
        this->OnKeyPressed(keyCode, event);
    };
	listener->onKeyReleased = [this](WinKeyCode keyCode, cocos2d::Event* event) {
        this->OnKeyRelease(keyCode, event);
    };

	auto eventDispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
	eventDispatcher->addEventListenerWithSceneGraphPriority(listener, node);
}

void UserInputHandler::OnKeyPressed(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const auto input = Input(keyCode);
    auto body = m_model->GetBody();
    if(input.jump && body->CanJump()) {
        body->Jump();
    }
    else if( input.dx == 1) {
        body->MoveRight();
    }
    else if( input.dx == -1) {
        body->MoveLeft();
    }
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    // TODO: stop if it doesn't move neither left nor right 
}