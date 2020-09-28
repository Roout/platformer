#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Unit.hpp"
#include <memory>

class SmoothFollower;
class UserInputHandler;

class Player final : public Unit {
public:

    inline static constexpr float DAMAGE_ON_CONTACT { 3.f }; 

    static Player* create();

    [[nodiscard]] bool init() override;

    void update(float dt) override;

    void setPosition(const cocos2d::Vec2& position) override;
    
    void pause() override;

private:
    enum class State {
        IDLE,
        WALK,
        JUMP,
        ATTACK,
        DEAD,
        COUNT
    };

    std::string GetStateName(Player::State state);

    void UpdateDebugLabel() noexcept;

    void UpdateAnimation() override;

    void UpdatePosition(const float dt) noexcept override;

    void UpdateState(const float dt) noexcept override;

    void OnDeath() override;
 
    void AddPhysicsBody() override;

    void AddWeapon() override;

    void AddAnimator() override;

    void MoveAlong(float x, float y) noexcept override;

    Player();

    /// Properties
private:
    std::unique_ptr<SmoothFollower> m_follower { nullptr };

    // controller:
    std::unique_ptr<UserInputHandler> m_inputHandler { nullptr };

    State m_currentState  { State::IDLE };

    State m_previousState { State::IDLE };
};


#endif // PLAYER_HPP