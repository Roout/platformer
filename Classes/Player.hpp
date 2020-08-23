#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Unit.hpp"
#include <memory>

class SmoothFollower;

class Player final : public Unit {
public:
    static constexpr char * const NAME = "Player";

    static Player* create(const cocos2d::Size&);

    [[nodiscard]] bool init() override;

    void update(float dt) override;

    void setPosition(const cocos2d::Vec2& position) override;

    void UpdateState(const float dt) noexcept override;

private:
    Player(const cocos2d::Size&);

    std::unique_ptr<SmoothFollower> m_follower { nullptr };
};

#endif // PLAYER_HPP