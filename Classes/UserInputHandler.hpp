#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "CCEventKeyboard.h"
#include <memory>
#include <array>

// This is player's avatar
class Unit; 

using WinKeyCode = cocos2d::EventKeyboard::KeyCode;

// used as controller
// for now it need to live at least as much as player
class UserInputHandler final {
    // Lifetime management
public:
    // define which unit&node it listen to.
    UserInputHandler(Unit * const);
    ~UserInputHandler() = default;

    UserInputHandler(const UserInputHandler& ) = delete;
    UserInputHandler& operator=(const UserInputHandler& ) = delete;

    UserInputHandler(UserInputHandler&& ) = delete;
    UserInputHandler& operator=(UserInputHandler&& ) = delete;

    /**
     * Force to release all buttons.
     */
    void Reset();
private:
    // callbacks 
    void OnKeyPressed(WinKeyCode keyCode, cocos2d::Event* event);
    
    void OnKeyRelease(WinKeyCode keyCode, cocos2d::Event* event);

private:
    /** @brief
     * Used to keep track of user's last input
    */
    struct Input final {
        bool jump { false };
        bool attack { false };
        int dx { 0 };

        Input() = default;
        
        static Input Create(WinKeyCode) noexcept;
        
        /** @brief
         * Update input state:
         * - always change old jump state to new one;
         * - change direction (@dx) only when new input was given. 
        */
        void Merge(const Input&) noexcept;
    };

    Input m_lastInput {};
    Unit * const m_player { nullptr };
    std::array<WinKeyCode, 8U> m_validKeys;
};

#endif // USER_INPUT_H