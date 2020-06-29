#include "UserInputHandler.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"

UserInputHandler::Input UserInputHandler::Input::Create(WinKeyCode keyCode) noexcept {
    Input input;
    if (keyCode == WinKeyCode::KEY_LEFT_ARROW ||
        keyCode == WinKeyCode::KEY_A
    ) {
        input.dx = -1;
    }
    else if (keyCode == WinKeyCode::KEY_RIGHT_ARROW ||
            keyCode == WinKeyCode::KEY_D
    ) {
        input.dx = 1;
    }
    
    if (keyCode == WinKeyCode::KEY_UP_ARROW ||
        keyCode == WinKeyCode::KEY_W || 
        keyCode == WinKeyCode::KEY_SPACE
    ) {
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
    m_model { model },
    m_movement { std::make_unique<Movement>(model) }
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
    const auto body = m_model->GetBody();
    static constexpr float EPS { 0.000001 };
    if(m_lastInput.jump && helper::IsEquel(const_cast<cocos2d::PhysicsBody*>(body)->getVelocity().y, 0.f, EPS) ) {
        m_movement->Jump();
    }

    if( m_lastInput.dx == 1) {
        m_movement->MoveRight();
    }
    else if( m_lastInput.dx == -1) {
        m_movement->MoveLeft();
    }

    if( m_lastInput.attack) {
        m_model->MeleeAttack();
    }
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const Input released { Input::Create(keyCode) };
    // TODO: release up and left/right is ongoing!
    if(released.dx && released.dx == m_lastInput.dx) {
        m_movement->StopXAxisMove();
        m_lastInput.dx = 0;
    }

    if(released.attack) {
        m_lastInput.attack = false;
    }
}