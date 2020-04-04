#ifndef UNIT_H
#define UNIT_H

class KinematicBody;
class PhysicWorld;

namespace cocos2d {
    class DrawNode; 
}

class Unit final { 
public:
    Unit(PhysicWorld *world, float x, float y);

    ~Unit();

    [[nodiscard]] KinematicBody * GetBody() noexcept {
        return m_body;
    }

    [[nodiscard]] const KinematicBody * GetBody() const noexcept {
        return m_body;
    }
private:
    // banch of just observer pointers.
    PhysicWorld * const         m_physicWorld { nullptr };
    KinematicBody * const       m_body { nullptr };

    static constexpr float m_width { 16.f };
    static constexpr float m_height { 28.f };
};

#endif // UNIT_H