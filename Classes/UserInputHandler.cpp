#include "UserInputHandler.h"
#include "Unit.h"
#include "PhysicWorld.h" // KinematicBody access

#include "cocos2d.h"

UserInputHandler::Input UserInputHandler::Input::Create(WinKeyCode keyCode) noexcept {
    Input input;
    if (keyCode == WinKeyCode::KEY_LEFT_ARROW ||
        keyCode == WinKeyCode::KEY_A) 
    {
        input.dx = -1;
    }
    else if (keyCode == WinKeyCode::KEY_RIGHT_ARROW ||
            keyCode == WinKeyCode::KEY_D)
    {
        input.dx = 1;
    }
    else if (keyCode == WinKeyCode::KEY_UP_ARROW ||
            keyCode == WinKeyCode::KEY_W || 
            keyCode == WinKeyCode::KEY_SPACE)
    {
        input.jump = true;
    }
    return input;
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
    m_lastInput = Input::Create(keyCode);
    auto body = m_model->GetBody();
    if(m_lastInput.jump && body->CanJump()) {
        body->Jump();
    }
    else if( m_lastInput.dx == 1) {
        body->MoveRight();
    }
    else if( m_lastInput.dx == -1) {
        body->MoveLeft();
    }
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const Input released{ Input::Create(keyCode)};

    if(released.dx == m_lastInput.dx) {
        auto body = m_model->GetBody();
        body->Stop();
    }
}