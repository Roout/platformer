#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Unit.hpp"
#include <memory>

class SmoothFollower;

class Player final : public Unit {
public:

    inline static constexpr float DAMAGE_ON_CONTACT { 7.f }; 

    static Player* create();

    [[nodiscard]] bool init() override;

    void update(float dt) override;

    void setPosition(const cocos2d::Vec2& position) override;
    
    void pause() override;

    void RecieveDamage(int damage) noexcept override;

    bool IsInvincible() const noexcept;

private:
    enum class State {
        IDLE,
        WALK,
        JUMP,
        MELEE_ATTACK,
        RANGE_ATTACK,
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

    void AddWeapons() override;

    void RangeAttack();

    void MeleeAttack();

    void AddAnimator() override;

    void MoveAlong(float x, float y) noexcept override;

    Player();

    /// Properties
private:
    enum WeaponClass { MELEE, RANGE };

    std::unique_ptr<SmoothFollower> m_follower { nullptr };

    // controller:
    friend class UserInputHandler;
    
    std::unique_ptr<UserInputHandler> m_inputHandler { nullptr };

    State m_currentState  { State::IDLE };

    State m_previousState { State::IDLE };

    bool m_isInvincible { false };
};


#endif // PLAYER_HPP