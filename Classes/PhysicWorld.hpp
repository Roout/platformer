#ifndef OWN_PHYSIC_WORLD_HPP
#define OWN_PHYSIC_WORLD_HPP

#include "math/CCGeometry.h"  // cocos2d::Vec2, cocos2d::Size
#include <functional>   // std::function used for callback type
#include <vector>
#include <utility>      // std::pair 
#include <optional> 
#include <algorithm>    // std::find_if
#include <utility>      // std::swap
#include <cassert>
#include <memory>       // std::unique_ptr
#include <type_traits>  // std::is_same_v, std::enable_if
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
     * Provide a set of the methods which are used on collision, e.g. 
     * recieving the damage.
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

    StaticBody& operator= (const StaticBody&) = default;
    StaticBody& operator= (StaticBody&&)  = default;

    virtual ~StaticBody() = default;
public:

    StaticBody (
        const cocos2d::Vec2& position, 
        const cocos2d::Size& size,
        core::Entity * const model
    ) :
        m_shape { position, size },
        m_model { model }
    {}

    [[nodiscard]] bool CanInteract( const StaticBody* const other ) const noexcept {
        return  (m_categoryBits & other->m_collideWithBits) != 0 &&
                (m_collideWithBits & other->m_categoryBits) != 0;
    }

    [[nodiscard]] bool Collide( const StaticBody *const body ) const noexcept {
        return m_shape.intersectsRect(body->m_shape);
    }

    void SetMask( MaskType categoryBits, MaskType collideMask ) noexcept {
        m_categoryBits = categoryBits;
        m_collideWithBits = collideMask;
    }

    [[nodiscard]] const cocos2d::Rect& GetShape() const noexcept {
        return m_shape;
    }

    [[nodiscard]] bool HasModel() const noexcept {
        return m_model != nullptr;
    }

    /**
     * This method invokes 'callback' function passing 
     * this entity's model to it as a parametr. 
     * 
     * Created to avoid giving access to private data member 'm_model'.
     * 
     * @param[in] callback
     *      The callback function passed via universal reference.
     *      It must be convertiable to std::function<void(core::Entity*)>. 
     *  
     * @note
     *      Callback function must have check for a valid 
     *      model pointer (not nullptr). 
     *      Possible side effect: change of the state of the 
     *      model pointed by 'm_model' pointer.   
     */
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

    // Methods used by models (unit, projectile, etc)

    void SetXAxisSpeed(float speed) noexcept {
        m_speed = speed;
    }    

    void SetDirection(const cocos2d::Vec2& direction) noexcept {
        m_direction = direction;
    }

    cocos2d::Vec2 GetDirection() const noexcept {
        return m_direction;
    }

    // Methods used by controllers (* input handler)

    void Jump() noexcept {
        m_direction.y = 1.f;
        m_jumpTime = JUMP_TIME;
    }

    void MoveLeft() noexcept {
        m_direction.x = -1.f;
    }

    void MoveRight() noexcept {
        m_direction.x = 1.f;
    }

    void Stop() noexcept {
        m_direction.x = 0.f;
    }

    [[nodiscard]] bool IsFallingDown() const noexcept {
        return m_direction.y < 0.f;
    }

    [[nodiscard]] bool CanJump() const noexcept {
        static constexpr auto error { 0.00001f };
        return (
            m_jumpTime <= 0.f && // not in jump state
            std::fabs(m_previousPosition.y - m_shape.origin.y) <= error // wasn't moved along Y-axis last iteration
        );
    }

    // Methods used by PhysicWorld in movement&collision implementation.
private:
    friend class PhysicWorld;
    
    void MoveX(float dt) noexcept {
        m_previousPosition.x = m_shape.origin.x;
        m_shape.origin.x += m_direction.x * m_speed * dt;
    }

    void MoveY(float dt) noexcept {
        m_previousPosition.y = m_shape.origin.y;
        m_shape.origin.y += m_direction.y * dt * JUMP_SPEED;

        m_previousJumpTime = m_jumpTime;

        if( m_jumpTime > 0.f ) { 
            // jumping
            m_jumpTime -= dt;
        }
        if( m_jumpTime <= 0.f ) {
            // the jump time limit is over, now fall down 
            m_direction.y = -1.f;
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
        m_direction.y = -1.f;
        m_jumpTime = 0.f;
    }

    // Properties
 private:   
    /**
     * This is x-axis speed of the body. 
     */
    float           m_speed { MOVE_SPEED };
    /**
     * Define movement direction of the body. 
     */
    cocos2d::Vec2   m_direction { 0.f, -1.f };
    /**
     * Save previous position of the body after movement. 
     * 
     * @note
     *      Current position is defined by StaticBody::m_shape.origin 
     *      as left-bottom corner.
     */
    cocos2d::Vec2   m_previousPosition { 0.f, 0.f };
    float           m_jumpTime { 0.f }; 
    float           m_previousJumpTime { 0.f };

    static constexpr float JUMP_SPEED { 120.f };
    static constexpr float JUMP_TIME  { 0.55f };
    static constexpr float MOVE_SPEED { 110.f };
};

class PhysicWorld final {
public:
    /**
     * Callback function type. 
     * Define what the callback owner will do with entity (passed to callback as parametr)
     * when the collision occur.
     */
    using OnCollision = std::function<void(core::Entity *)>;
    
    void Step(const float dt, size_t iterations);

    template<class BodyType> 
    [[nodiscard]] BodyType * Create(
        const cocos2d::Vec2& position,
        const cocos2d::Size& size,
        core::Entity * const model = nullptr, 
        std::optional<OnCollision> callback = std::nullopt
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
    std::optional<OnCollision> callback
) {
    static_assert(
            std::is_same_v<BodyType, StaticBody> || 
            std::is_same_v<BodyType, KinematicBody>,
            "wrong body type" 
    );

    if constexpr (std::is_same_v<BodyType, StaticBody>) {
        return &(m_staticBodies.emplace_back( 
            BodyType{ position, size, model }, // body  
            callback 
        ).first);
    }
    else {
        return &(m_kinematicBodies.emplace_back( 
            BodyType{ position, size, model }, // body  
            callback 
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
        auto it = std::find_if(container.begin(), container.end(), [body](const MyPair<BodyType>& cPair) {
            return (body == &cPair.first);
        });
        assert(it != container.end());
        // Order doesn't matter so I can just swap and pop element
        const auto backIt = --container.cend();
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

/**
 * This namespace keep all stuff that somehow connected with 
 * memory management. 
 */
namespace factory 
{   
    template<class Ty>
    struct body_deleter final {
        using body_type = Ty;

        body_deleter(PhysicWorld * const world) : 
            m_world { world } 
        { // default ctor
        }
        
        body_deleter() = default;

        /**
         * This function erases already registered body 
         * from the physic world 'm_world'.
         */
        void operator() ( body_type * const body) noexcept {
            m_world->Erase(body);
        }

    private:
        PhysicWorld * const m_world { nullptr };
    };

    /**
     * This is a template of the functional object's class that allows to
     * create and register physcal bodies comfortably at one place. Also 
     * class user doesn't need to care about object destruction.
     */
    template<
        class Ty, 
        class = std::enable_if<
            std::is_same_v<Ty, StaticBody> || 
            std::is_same_v<Ty, KinematicBody>
        >
    >
    struct body_creator final {
        using body_type = Ty;
        using body_ptr  = std::unique_ptr<body_type, body_deleter<body_type>>;

        body_creator(PhysicWorld * const world) : 
            m_world { world } 
        { // default ctor
        }
        
        body_creator() = default;
        
        /**
         * This is template function that creates and register in physical world
         * the physcial body.
         * 
         * @return
         *      std::unique_ptr to created physcal body of 'body_type' with deleter as 
         *      part of pointer type.
         */
        template<class ... Args>
        [[nodiscard]] body_ptr operator() (Args&& ... args) noexcept {
            return { 
                m_world->Create<body_type>(std::forward<Args>(args)...), 
                body_deleter<body_type>{ m_world } 
            };
        }

    private:

        PhysicWorld * const m_world { nullptr };
    };

}

/**
 * Handy alias used outside this code section. 
 * Hide from user factory::body_deleter<> and 
 * verbose pointer declaration.   
 */
template<
    class Ty, 
    class = std::enable_if<
        std::is_same_v<Ty, StaticBody> || 
        std::is_same_v<Ty, KinematicBody>
    >
>
using BodyPointer = std::unique_ptr<Ty, factory::body_deleter<Ty>>;

#endif // OWN_PHYSIC_WORLD_HPP