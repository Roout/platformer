#ifndef OWN_PHYSIC_WORLD_HPP
#define OWN_PHYSIC_WORLD_HPP

#include "cocos/math/CCGeometry.h"  // cocos2d::Vec2, cocos2d::Size
#include <functional>   // std::function used for callback type
#include <vector>
#include <utility>      // std::pair 
#include <optional> 
#include <algorithm>    // std::find_if
#include <utility>      // std::swap
#include <cassert>
#include <type_traits>  // std::is_same_v
#include <cmath>        // std::fabs

/**
 * Underlying type of bitmask used to represent 
 * entity's category.
 */
using MaskType = uint16_t;

enum class CategoryBits: MaskType {
    HERO        = 0x0001,
    ENEMY       = 0x0002,
    BOUNDARY    = 0x0004
};

template<class ...Args>
constexpr MaskType CreateMask(Args ... bits) noexcept {
    return (static_cast<MaskType>(bits) | ...);
} 

namespace core {
    /**
     * Forward declaration of the base class of all entities in this game.
     * Provide a set of the methods which are used on collision, e.g. LoseHealth().
     */
    class Entity;
}

class StaticBody {
public:
    /**
     * @note 
     *      Copy & Move constructors and assignment are breaking logic: 
     *      there will be several bodies of the only one model. 
     */
    StaticBody(StaticBody&&) = default;
    StaticBody(const StaticBody&) = default;

    StaticBody& operator=(const StaticBody&) = default;
    StaticBody& operator=(StaticBody&&)  = default;

    virtual ~StaticBody() = default;
public:

    StaticBody (
        const cocos2d::Vec2& position, 
        const cocos2d::Size& size,
        core::Entity * const model
    ) :
        m_shape{ position, size },
        m_model{ model }
    {}

    [[nodiscard]] bool CanInteract(const StaticBody* const other ) const noexcept {
        return  (m_categoryBits & other->m_collideWithBits) != 0 &&
                (m_collideWithBits & other->m_categoryBits) != 0;
    }

    [[nodiscard]] bool Collide(const StaticBody *const body) const noexcept {
        return m_shape.intersectsRect(body->m_shape);
    }

    void SetMask(MaskType categoryBits, MaskType collideMask) noexcept {
        m_categoryBits = categoryBits;
        m_collideWithBits = collideMask;
    }

    [[nodiscard]] const cocos2d::Rect& GetShape() const noexcept {
        return m_shape;
    }

    // [[nodiscard]] core::Entity* GetModel() const noexcept {
    //     return m_model;
    // }

    [[nodiscard]] bool HasModel() const noexcept {
        return m_model != nullptr;
    }

    template<class Callable>
    void InvokeCallback(Callable&& callback) noexcept {
        std::invoke(callback, m_model);
    }
protected:
    MaskType        m_categoryBits { 0 };       // I am a ... .
    MaskType        m_collideWithBits { 0 };    // I collide with a ... .
    cocos2d::Rect   m_shape {};
    /**
     * Define the view pointer of the model this body belong to.
     * Used by callback on collision.
     */
    core::Entity * m_model { nullptr }; 
};

class KinematicBody final : 
    public StaticBody 
{   
public:
    using StaticBody::StaticBody;

    void MoveX(float dt) noexcept {
        m_previousPosition.x = m_shape.origin.x;
        m_shape.origin.x += m_velocity.x * MOVE_SPEED * dt;
    }
    void MoveY(float dt) noexcept {
        m_previousPosition.y = m_shape.origin.y;
        m_shape.origin.y += m_velocity.y * dt * JUMP_SPEED;

        m_previousJumpTime = m_jumpTime;

        if( m_jumpTime > 0.f ){ 
            // jumping
            m_jumpTime -= dt;
        }
        if( m_jumpTime <= 0.f ) {
            // the jump time limit is over, now fall down 
            m_velocity.y = -1.f;
        }
    }
    
    void RestoreX() noexcept {
        m_shape.origin.x = m_previousPosition.x;
    }
    void RestoreY() noexcept {
        m_jumpTime = m_previousJumpTime;
        m_shape.origin.y = m_previousPosition.y;
    }

    
    void StartFall() noexcept {
        m_velocity.y = -1.f;
        m_jumpTime = 0.f;
    }
    void Jump() noexcept {
        m_velocity.y = 1.f;
        m_jumpTime = JUMP_TIME;
    }
    void MoveLeft() noexcept {
        m_velocity.x = -1.f;
    }
    void MoveRight() noexcept {
        m_velocity.x = 1.f;
    }
    void Stop() noexcept {
        m_velocity.x = 0.f;
    }

    [[nodiscard]] bool IsFallingDown() const noexcept {
        return m_velocity.y < 0.f;
    }
    [[nodiscard]] bool CanJump() const noexcept {
        static constexpr auto error { 0.00001f };
        return (
            m_jumpTime <= 0.f && // not in jump state
            std::fabs(m_previousPosition.y - m_shape.origin.y) <= error // wasn't moved along Y-axis last iteration
        );
    }
    
private:
    cocos2d::Vec2   m_velocity { 0.f, -1.f };
    // current position is defined by StaticBody::m_shape.origin as left-bottom corner
    cocos2d::Vec2   m_previousPosition { 0.f, 0.f };
    float           m_jumpTime { 0.f }; 
    float           m_previousJumpTime { 0.f };

    static constexpr float JUMP_SPEED { 120.f };
    static constexpr float MOVE_SPEED { 110.f };

    static constexpr float JUMP_TIME  { 0.55f };
};

class PhysicWorld final {
public:
    /**
     * todo describe this function
     */
    using OnCollision = std::function<void(core::Entity *)>;
    
    void Step(const float dt, size_t iterations);

    template<class BodyType> 
    [[nodiscard]] BodyType * Create(
        const cocos2d::Vec2& position,
        const cocos2d::Size& size,
        core::Entity * const model = nullptr, 
        OnCollision * callback = nullptr
    );

    template<class BodyType> 
    void Erase(const BodyType* const body ) noexcept;

private:
    void Step(const float dt);

    template<class BodyType>
    using MyPair = std::pair<BodyType, std::optional<OnCollision>>;

    
    std::vector<MyPair<StaticBody>>     m_staticBodies;
    std::vector<MyPair<KinematicBody>>  m_kinematicBodies;
};

/////////////////////////////
// TEMPLATE IMPLEMENTATION //
/////////////////////////////
template<class BodyType> 
BodyType * PhysicWorld::Create(
    const cocos2d::Vec2& position,
    const cocos2d::Size& size,
    core::Entity * const model, 
    PhysicWorld::OnCollision * callback
) {
    static_assert(
            std::is_same_v<BodyType, StaticBody> || 
            std::is_same_v<BodyType, KinematicBody>,
            "wrong body type" 
    );

    if constexpr (std::is_same_v<BodyType, StaticBody>) {
        return &(m_staticBodies.emplace_back( 
            BodyType{ position, size, model }, // body  
            ( callback ? std::make_optional(*callback): std::nullopt ) 
        ).first);
    }
    else {
        return &(m_kinematicBodies.emplace_back( 
            BodyType{ position, size, model }, // body  
            ( callback ? std::make_optional(*callback): std::nullopt ) 
        ).first);
    }
}

template<class BodyType> 
void PhysicWorld::Erase(const BodyType* const body ) noexcept {
    static_assert(
            std::is_same_v<BodyType, StaticBody> || 
            std::is_same_v<BodyType, KinematicBody>,
            "wrong body type" 
    );

    auto Remove = [body](std::vector<MyPair<BodyType>>& container) {
        auto it = std::find_if(container.begin(), container.end(), [body](const MyPair<BodyType>& cPair){
            return (body == &cPair.first);
        });
        assert(it != container.end());
        // Order doesn't matter so I can just swap and pop element
        const auto backIt = --container.end();
        // discard element pointed by iterator 'it'
        *it = *backIt;
        container.pop_back();
    };

    if constexpr (std::is_same_v<BodyType, StaticBody>) {
        Remove(m_staticBodies);
    }
    else {
        Remove(m_kinematicBodies);
    }
}

#endif // OWN_PHYSIC_WORLD_HPP