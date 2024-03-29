#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Unit.hpp"

#include <memory>

namespace json_autogenerated_classes {
    struct Player;
} // namespace json_autogenerated_classes
namespace json_models = json_autogenerated_classes;

class SmoothFollower;
class Dash;

class Player final : public Unit {
public:
    static constexpr float DAMAGE_ON_CONTACT { 7.f }; 

    static Player* create(const cocos2d::Size& contentSize
        , const json_models::Player *model);

    [[nodiscard]] bool init() override;

    void update(float dt) override;

    void setPosition(const cocos2d::Vec2& position) override;
    
    void pause() override;

    void RecieveDamage(int damage) noexcept override;

private:
    enum class State {
        IDLE,
        DEAD,
        WALK,
        JUMP,
        DASH,
        MELEE_ATTACK_1, // simple attack
        MELEE_ATTACK_2, // simple attack
        MELEE_ATTACK_3, // simple attack
        RANGE_ATTACK, // fireball attack
        PREPARE_RANGE_ATTACK,
        SPECIAL_PHASE_1,
        SPECIAL_PHASE_2,
        SPECIAL_PHASE_3,
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

    void InitiateDash();

//==== Special Attack
    void SpecialAttack();

    void StartSpecialAttack();

    void OnSpecialAttackEnd();

    void FinishSpecialAttack();
//===

    void Attack() override;

    void AddAnimator() override;

    // TODO: maybe seperate jumping/falling with simple right\left move
    void MoveAlong(Movement::Direction) noexcept override;

    Player(const cocos2d::Size& contentSize
        , const json_models::Player *model);

    /// Properties
private:
    enum WeaponClass { MELEE, RANGE, SPECIAL };

    std::unique_ptr<SmoothFollower> m_follower { nullptr };

    // controller:
    friend class UserInputHandler;
    
    std::unique_ptr<UserInputHandler> m_inputHandler { nullptr };

    State m_currentState  { State::IDLE };

    State m_previousState { State::IDLE };

    bool m_scheduleSpecialAttack { false };

    bool m_finishSpecialAttack { false };

    Dash * m_dash { nullptr };

/// Configs (Json Model Data):
    const json_models::Player * const m_model { nullptr };
};

#endif // PLAYER_HPP