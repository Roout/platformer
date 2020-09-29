#include "UserInputHandler.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"
#include "UnitMovement.hpp"

#include <algorithm> // std::find

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

UserInputHandler::UserInputHandler(Unit * const player) :
    m_player { player },
    m_validKeys {
        WinKeyCode::KEY_LEFT_ARROW,
        WinKeyCode::KEY_A,
        WinKeyCode::KEY_RIGHT_ARROW,
        WinKeyCode::KEY_D,
        WinKeyCode::KEY_UP_ARROW,
        WinKeyCode::KEY_W,
        WinKeyCode::KEY_SPACE,
        WinKeyCode::KEY_F
    }
{
    const auto listener = cocos2d::EventListenerKeyboard::create();
    
    listener->onKeyPressed = [this](WinKeyCode keyCode, cocos2d::Event* event) {
        if(auto it = std::find(m_validKeys.cbegin(), m_validKeys.cend(), keyCode); 
            it != m_validKeys.cend() && !m_player->IsDead()
        ) {
            this->OnKeyPressed(keyCode, event);
        }
    };
	listener->onKeyReleased = [this](WinKeyCode keyCode, cocos2d::Event* event) {
        if(auto it = std::find(m_validKeys.cbegin(), m_validKeys.cend(), keyCode); 
            it != m_validKeys.cend() && !m_player->IsDead()
        ) {
            this->OnKeyRelease(keyCode, event);
        }
    };

	const auto eventDispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
	eventDispatcher->addEventListenerWithSceneGraphPriority(listener, player);
}

void UserInputHandler::Reset() {
    m_player->MoveAlong(0.f, 0.f);
    m_lastInput.dx = 0;
    m_lastInput.jump = false;
    m_lastInput.attack = false;
}

void UserInputHandler::OnKeyPressed(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    m_lastInput.Merge(Input::Create(keyCode));

    if(m_lastInput.jump && m_player->IsOnGround() ) {
        m_player->MoveAlong(0.f, 1.f);
    }

    if(m_lastInput.dx == 1) {
        m_player->MoveAlong(0.f, 0.f);
        m_player->MoveAlong(1.f, 0.f);
        if(m_player->IsLookingLeft()) {
            m_player->Turn();
        }
    }
    else if(m_lastInput.dx == -1) {
        m_player->MoveAlong(0.f, 0.f);
        m_player->MoveAlong(-1.f, 0.f);
        if(!m_player->IsLookingLeft()) {
            m_player->Turn();
        }
    }

    if(m_lastInput.attack) {
        m_player->Attack();
    }
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const Input released { Input::Create(keyCode) };
    // TODO: release up and left/right is ongoing!
    if(released.dx && released.dx == m_lastInput.dx) {
        m_player->MoveAlong(0.f, 0.f);
        m_lastInput.dx = 0;
    }

    if(released.attack) {
        m_lastInput.attack = false;
    }
}