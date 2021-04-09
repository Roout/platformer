#include "UserInputHandler.hpp"
#include "DragonBonesAnimator.hpp"
#include "PhysicsHelper.hpp"
#include "Movement.hpp"
#include "Dash.hpp"
#include "Utils.hpp"

#include "units/Player.hpp"

#include "cocos2d.h"

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
    
    if (keyCode == WinKeyCode::KEY_F) {
        input.meleeAttack = true;
    }
    else if (keyCode == WinKeyCode::KEY_G) {
        input.rangeAttack = true;
    }
    else if (keyCode == WinKeyCode::KEY_E) {
        input.specialAttack = true;
    }
    else if (keyCode == WinKeyCode::KEY_Q) {
        input.dash = true;
    }
    
    return input;
}

void UserInputHandler::Input::Merge(const Input& input) noexcept {
    jump = input.jump;
    meleeAttack = input.meleeAttack;
    rangeAttack = input.rangeAttack;
    specialAttack = input.specialAttack;
    dash = input.dash;
    if (input.dx == dx) {
        dx += input.dx;
    }
    else if (input.dx) {
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
        WinKeyCode::KEY_F, // simple sword attack
        WinKeyCode::KEY_G, // fireball
        WinKeyCode::KEY_Q, // dash
        WinKeyCode::KEY_E  // special attack
    }
{
    const auto listener = cocos2d::EventListenerKeyboard::create();
    
    listener->onKeyPressed = [this](WinKeyCode keyCode, cocos2d::Event* event) {
        if (auto it = std::find(m_validKeys.cbegin(), m_validKeys.cend(), keyCode); 
            it != m_validKeys.cend() && !m_player->IsDead()
        ) {
            this->OnKeyPressed(keyCode, event);
        }
    };
	listener->onKeyReleased = [this](WinKeyCode keyCode, cocos2d::Event* event) {
        if (auto it = std::find(m_validKeys.cbegin(), m_validKeys.cend(), keyCode); 
            it != m_validKeys.cend() && !m_player->IsDead()
        ) {
            this->OnKeyRelease(keyCode, event);
        }
    };

	const auto eventDispatcher = cocos2d::Director::getInstance()->getEventDispatcher();
	eventDispatcher->addEventListenerWithSceneGraphPriority(listener, player);
}

void UserInputHandler::Reset() {
    m_lastInput.dx = 0;
    m_lastInput.jump = 0;
    m_lastInput.meleeAttack = false;
    m_lastInput.rangeAttack = false;
    m_lastInput.specialAttack = false;
    m_lastInput.dash = false;
}

void UserInputHandler::OnKeyPressed(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const auto lastInputCopy { m_lastInput };
    m_lastInput.Merge(Input::Create(keyCode));

    const bool canBeControlled {
        (m_player->m_currentState != Player::State::DASH)
        && !m_lastInput.dash
    };

    /**
     * first jump from the ground   - m_lastInput.jump && m_player->IsOnGround()
     * first jump from the air      - m_lastInput.jump && m_lastInput.jumpCounter < 2
     * second jump from the air     - /
     */
    if (m_player->IsOnGround()) {
        // player make on the ground (doesn't matter which)
        // so the counter is reseted
        m_lastInput.jumpCounter = 0;
    }
    
    if ((m_lastInput.jump && m_player->IsOnGround()) || 
        (m_lastInput.jump && m_lastInput.jumpCounter < MAX_JUMP_COUNT)
    ) {
        if (canBeControlled) {
            m_player->MoveAlong(0.f, 1.f);
            if (m_player->IsOnGround()) {
                m_lastInput.jumpCounter = 1;
            }
            else {
                // jump in the air - first jump or second - doesn't matter
                m_lastInput.jumpCounter = MAX_JUMP_COUNT;
            }
        }
    }

    if (m_lastInput.dx > 0 && canBeControlled) {
        // reset only dx!
        m_player->FinishSpecialAttack();
        // Don't reset if player is moving in the same direction 
        const auto xVel = m_player->getPhysicsBody()->getVelocity().x;
        if(xVel < 0.f) {
            // Player is moving in other direction
            m_player->m_movement->ResetForceX();
        }
        // no need to reset the forceX!
        m_player->MoveAlong(1.f, 0.f);
        if (m_player->IsLookingLeft()) {
            m_player->Turn();
        }
    }
    else if (m_lastInput.dx < 0 && canBeControlled) {
        // reset only dx!
        m_player->FinishSpecialAttack();
        // Don't reset if player is moving in the same direction 
        const auto xVel = m_player->getPhysicsBody()->getVelocity().x;
        if(xVel > 0.f) {
            // Player is moving in other direction
            m_player->m_movement->ResetForceX();
        }
        m_player->MoveAlong(-1.f, 0.f);
        if (!m_player->IsLookingLeft()) {
            m_player->Turn();
        }
    }

    if (m_lastInput.dash) {
        m_player->InitiateDash();
    }
    else if (m_lastInput.meleeAttack && canBeControlled) {
        m_player->MeleeAttack();
    }
    else if (m_lastInput.rangeAttack && canBeControlled) {
        m_player->RangeAttack();
    }
    else if (m_lastInput.specialAttack && canBeControlled) {
        m_player->StartSpecialAttack();
    }
}

static inline void Decrease(int& value) noexcept {
    if(value > 0) value--;
    else if(value < 0) value++;
}

static inline bool HasSameSigh(int lhs, int rhs) noexcept {
    return (lhs >= 0 && rhs >= 0) || (lhs < 0 && rhs < 0);
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const auto lastInputCopy { m_lastInput };
    const auto released { Input::Create(keyCode) };
    const bool dashed { m_player->m_currentState == Player::State::DASH };

    if (released.dx && HasSameSigh(released.dx, m_lastInput.dx)) {
        Decrease(m_lastInput.dx);
        // reset only dx!
        if (dashed) {
            m_player->m_dash->ResetSavedBodyState();
        }
        else if(!m_lastInput.dx) {
            m_player->m_movement->ResetForceX();
        }
    }

    if (released.meleeAttack) {
        m_lastInput.meleeAttack = false;
    }
    else if (released.rangeAttack) {
        m_lastInput.rangeAttack = false;
    }
    else if (released.dash) {
        m_lastInput.dash = false;
    }
    else if (released.specialAttack) {
        m_lastInput.specialAttack = false;
        m_player->FinishSpecialAttack();
    }
}