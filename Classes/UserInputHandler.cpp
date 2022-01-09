#include "UserInputHandler.hpp"
#include "PhysicsHelper.hpp"
#include "Utils.hpp"

#include "components/DragonBonesAnimator.hpp"
#include "components/Movement.hpp"
#include "components/Dash.hpp"

#include "units/Player.hpp"

#include "cocos2d.h"

#include <algorithm> // std::find

namespace {

    inline bool HasSameSigh(int lhs, int rhs) noexcept {
        return (lhs >= 0 && rhs >= 0) || (lhs < 0 && rhs < 0);
    }

} // namespace {

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
    using Move = Movement::Direction;

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
    
    bool jumpFromGround { m_lastInput.jump && m_player->IsOnGround() };
    bool jumpFromAir { m_lastInput.jump 
        && m_lastInput.jumpCounter < MAX_JUMP_COUNT };

    if (canBeControlled && (jumpFromGround || jumpFromAir)) {
        m_player->MoveAlong(Move::UP);
        if (m_player->IsOnGround()) {
            m_lastInput.jumpCounter = 1;
        }
        else {
            // jump in the air - first jump or second - doesn't matter
            m_lastInput.jumpCounter = MAX_JUMP_COUNT;
        }
    }

    if (m_lastInput.dx && canBeControlled) {
        bool goLeft { m_lastInput.dx < 0 };
        bool goRight { m_lastInput.dx > 0 };
        // reset only dx!
        m_player->FinishSpecialAttack();
        // Don't reset if player is moving in the same direction 
        const auto xVel = m_player->getPhysicsBody()->getVelocity().x;
        if ((xVel < 0.f && goRight) || (xVel > 0.f && goLeft)) {
            // Player is moving in different direction
            m_player->Stop(Movement::Axis::X);
        }
        m_player->MoveAlong(goLeft? Move::LEFT: Move::RIGHT);
        if ((m_player->IsLookingLeft() && goRight)
            || (!m_player->IsLookingLeft() && goLeft)) 
        {
            m_player->Turn();
        }
    }

    if (m_lastInput.dash) {
        m_player->InitiateDash();
    }
    else if (canBeControlled) {
        if (m_lastInput.meleeAttack) {
            m_player->MeleeAttack();
        }
        else if (m_lastInput.rangeAttack) {
            m_player->RangeAttack();
        }
        else if (m_lastInput.specialAttack) {
            m_player->StartSpecialAttack();
        }
    }
}

void UserInputHandler::OnKeyRelease(
    WinKeyCode keyCode, 
    cocos2d::Event* event
) {
    const auto lastInputCopy { m_lastInput };
    const auto released { Input::Create(keyCode) };
    const bool dashed { m_player->m_currentState == Player::State::DASH };

    if (released.dx && HasSameSigh(released.dx, m_lastInput.dx)) {
        if (m_lastInput.dx > 0) {
            m_lastInput.dx--;
        }
        else if (m_lastInput.dx < 0) {
            m_lastInput.dx++;
        }

        if (dashed) {
            m_player->m_dash->ResetSavedBodyState();
        }
        else if (!m_lastInput.dx) {
            m_player->Stop(Movement::Axis::X);
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