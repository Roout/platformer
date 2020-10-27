#include "UserInputHandler.hpp"
#include "Player.hpp"
#include "PhysicsHelper.hpp"
#include "cocos2d.h"
#include "Movement.hpp"

#include <algorithm> // std::find

UserInputHandler::Input UserInputHandler::Input::Create(WinKeyCode keyCode) noexcept {
    Input input;
    if(keyCode == WinKeyCode::KEY_LEFT_ARROW ||
        keyCode == WinKeyCode::KEY_A
    ) {
        input.dx = -1;
    }
    else if(keyCode == WinKeyCode::KEY_RIGHT_ARROW ||
        keyCode == WinKeyCode::KEY_D
    ) {
        input.dx = 1;
    }
    
    if(keyCode == WinKeyCode::KEY_UP_ARROW ||
        keyCode == WinKeyCode::KEY_W || 
        keyCode == WinKeyCode::KEY_SPACE
    ) {
        input.jump = true;
    }
    
    if(keyCode == WinKeyCode::KEY_F) {
        input.meleeAttack = true;
    }
    else if(keyCode == WinKeyCode::KEY_G) {
        input.rangeAttack = true;
    }
    
    return input;
}

void UserInputHandler::Input::Merge(Input&& input) noexcept {
    jump = input.jump;
    meleeAttack = input.meleeAttack;
    rangeAttack = input.rangeAttack;
    if(input.dx) {
        dx = input.dx;
    }
}  

UserInputHandler::UserInputHandler(Player * const player) :
    m_player { player },
    m_validKeys {
        WinKeyCode::KEY_LEFT_ARROW,
        WinKeyCode::KEY_A,
        WinKeyCode::KEY_RIGHT_ARROW,
        WinKeyCode::KEY_D,
        WinKeyCode::KEY_UP_ARROW,
        WinKeyCode::KEY_W,
        WinKeyCode::KEY_SPACE,
        WinKeyCode::KEY_F,
        WinKeyCode::KEY_G
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
    m_player->m_movement->ResetForce();
    m_lastInput.dx = 0;
    m_lastInput.jump = 0;
    m_lastInput.meleeAttack = false;
    m_lastInput.rangeAttack = false;
}

void UserInputHandler::OnKeyPressed(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    m_lastInput.Merge(Input::Create(keyCode));
    /**
     * first jump from the ground   - m_lastInput.jump && m_player->IsOnGround()
     * first jump from the air      - m_lastInput.jump && m_lastInput.jumpCounter < 2
     * second jump from the air     - /
     */
    if(m_player->IsOnGround()) {
        // player make on the ground (doesn't matter which)
        // so the counter is reseted
        m_lastInput.jumpCounter = 0;
    }
    
    if( (m_lastInput.jump && m_player->IsOnGround()) || 
        (m_lastInput.jump && m_lastInput.jumpCounter < MAX_JUMP_COUNT)
    ) {
        m_player->MoveAlong(0.f, 1.f);

        if(m_player->IsOnGround()) {
            m_lastInput.jumpCounter = 1;
        }
        else {
            // jump in the air - first jump or second - doesn't matter
            m_lastInput.jumpCounter = MAX_JUMP_COUNT;
        }
    }

    if(m_lastInput.dx == 1) {
        // reset only dx!
        m_player->m_movement->ResetForceX();
        m_player->MoveAlong(1.f, 0.f);
        if(m_player->IsLookingLeft()) {
            m_player->Turn();
        }
    }
    else if(m_lastInput.dx == -1) {
        // reset only dx!
        m_player->m_movement->ResetForceX();
        m_player->MoveAlong(-1.f, 0.f);
        if(!m_player->IsLookingLeft()) {
            m_player->Turn();
        }
    }

    if(m_lastInput.meleeAttack) {
        m_player->MeleeAttack();
    }
    else if(m_lastInput.rangeAttack) {
        m_player->RangeAttack();
    }
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const Input released { Input::Create(keyCode) };
    // TODO: release up and left/right is ongoing!
    if(released.dx && released.dx == m_lastInput.dx) {
        // reset only dx!
        m_player->m_movement->ResetForceX();
        m_lastInput.dx = 0;
    }

    if(released.meleeAttack) {
        m_lastInput.meleeAttack = false;
    }
    else if(released.rangeAttack) {
        m_lastInput.rangeAttack = false;
    }
}