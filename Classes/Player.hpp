#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Unit.hpp"
#include <memory>

class SmoothFollower;
class UserInputHandler;

class Player final : public Unit {
public:
    static constexpr char * const NAME = "Player";

    static Player* create();

    [[nodiscard]] bool init() override;

    void update(float dt) override;

    void setPosition(const cocos2d::Vec2& position) override;
    
    void pause() override;

    void resume() override;

private:
    void UpdateState(const float dt) noexcept override;

    void UpdateAnimation() override;

    void AddPhysicsBody(const cocos2d::Size&) override;

    void AddAnimator() override;

    void UpdatePosition(const float dt) noexcept override;

    Player();

    /// Properties
private:
    std::unique_ptr<SmoothFollower> m_follower { nullptr };

    // controller:
    std::unique_ptr<UserInputHandler> m_inputHandler { nullptr };
};

#endif // PLAYER_HPP