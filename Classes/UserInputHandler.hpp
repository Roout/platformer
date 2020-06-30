#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "CCEventKeyboard.h"
#include <memory>

// This is player's avatar
class Unit; 
class Movement;
namespace cocos2d {
    class Node;
}

using WinKeyCode = cocos2d::EventKeyboard::KeyCode;
// used as controller
// note: must be destroyed before Unit
class UserInputHandler final {
public:
    // define which unit and which node it listen to.
    UserInputHandler(Unit * const, Movement * const, cocos2d::Node * const );
    ~UserInputHandler() = default;

    UserInputHandler(const UserInputHandler& ) = delete;
    UserInputHandler& operator=(const UserInputHandler& ) = delete;

    UserInputHandler(UserInputHandler&& ) = delete;
    UserInputHandler& operator=(UserInputHandler&& ) = delete;
    
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
    Unit * const m_model { nullptr };
    Movement * const m_movement { nullptr };
};

#endif // USER_INPUT_H