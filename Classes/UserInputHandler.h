#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "CCEventKeyboard.h"

// This is player's avatar
class Unit; 
namespace cocos2d {
    class Node;
}

using WinKeyCode = cocos2d::EventKeyboard::KeyCode;
// used as controller
// note: must be destroyed before Unit
class UserInputHandler final {
public:
    // define which unit and which node it listen to.
    UserInputHandler(Unit * const, cocos2d::Node * const );
    ~UserInputHandler() = default;

    UserInputHandler(const UserInputHandler& ) = delete;
    UserInputHandler&operator=(const UserInputHandler& ) = delete;
    
private:
    // callbacks 
    void OnKeyPressed(WinKeyCode keyCode, cocos2d::Event* event);
    
    void OnKeyRelease(WinKeyCode keyCode, cocos2d::Event* event);

private:
    struct Input final {
        bool jump { false };
        int dx { 0 };

        Input() = default;
        
        static Input Create(WinKeyCode) noexcept;
    };

    Input m_lastInput {};
    Unit * const m_model { nullptr };
};

#endif // USER_INPUT_H