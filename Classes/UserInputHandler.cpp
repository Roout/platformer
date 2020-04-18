#include "UserInputHandler.hpp"
#include "Unit.hpp"
#include "PhysicWorld.hpp" // KinematicBody access

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
    
    if (keyCode == WinKeyCode::KEY_UP_ARROW ||
            keyCode == WinKeyCode::KEY_W || 
            keyCode == WinKeyCode::KEY_SPACE)
    {
        input.jump = true;
    }
    
    if( keyCode == WinKeyCode::KEY_F ) {
        input.attack = true;
    }
    
    return input;
}

void UserInputHandler::Input::Merge(const Input& input) noexcept {
    jump = input.jump;
    attack = input.attack;
    if(input.dx) {
        dx = input.dx;
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
    m_lastInput.Merge(Input::Create(keyCode));
    auto body = m_model->GetBody();
    
    if(m_lastInput.jump && body->CanJump()) {
        body->Jump();
    }

    if( m_lastInput.dx == 1) {
        body->MoveRight();
    }
    else if( m_lastInput.dx == -1) {
        body->MoveLeft();
    }

    if( m_lastInput.attack) {
        m_model->MeleeAttack();
    }
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const Input released{ Input::Create(keyCode)};
    if(released.dx && released.dx == m_lastInput.dx) {
        auto body = m_model->GetBody();
        body->Stop();
        m_lastInput.dx = 0;
    }
    if(released.attack) {
        m_lastInput.attack = false;
    }
}