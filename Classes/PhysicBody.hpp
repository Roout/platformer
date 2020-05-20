#ifndef PHYSIC_BODY_HPP
#define PHYSIC_BODY_HPP

#include "math/CCGeometry.h"  // cocos2d::Vec2, cocos2d::Size
#include "Core.hpp"

/**
 * Underlying type of bitmask used to represent 
 * entity's category.
 */
using MaskType = uint16_t;

enum class CategoryBits: MaskType {
    HERO        = 0x0001,
    ENEMY       = 0x0002,
    BOUNDARY    = 0x0004,
    PROJECTILE  = 0x0008,
    PLATFORM    = 0x0010
};

template<class ...Args>
constexpr MaskType CreateMask(Args ... bits) noexcept {
    return (static_cast<MaskType>(bits) | ...);
} 

/**
 * It's a holder class which separate the user information
 * from the physic body types. 
 * 
 * If any of data member is initialized - all data members must be initialized
 */
struct Fixture final {
    /**
     * Define the view pointer of the model this body belong to.
     * Used by callback on collision.
     */
    core::Entity * const        m_entity { nullptr };
    /**
     * Indicate whoch category does the entity belong.
     */
    const core::CategoryName    m_category { core::CategoryName::UNDEFINED };


    // constructor:
    Fixture( core::Entity * const entity, core::CategoryName category) :
        m_entity { entity },
        m_category { category }
    {}
};

class StaticBody {
public:
    /**
     * @note 
     *      Copy & Move constructors and assignment somewhat breaks logic: 
     *      there will be several bodies of the only one model.
     *      However it is still acceptable and doesn't break destructor.
     */
    StaticBody(StaticBody&&) = default;
    StaticBody(const StaticBody&) = default;

    StaticBody& operator= (const StaticBody&) = default;
    StaticBody& operator= (StaticBody&&)  = default;

public:
    friend class PhysicWorld;
    
    StaticBody (
        const cocos2d::Vec2& position, 
        const cocos2d::Size& size
    ) :
        m_shape { position, size }
    {}

    virtual ~StaticBody() = default;

    /**
     * This method initializes user's data that needed to be used 
     * when handling collisions. 
     */
    void EmplaceFixture(
        core::Entity * const entity, 
        core::CategoryName category) noexcept 
    {
        m_fixture.emplace(entity, category);
    }

    /**
     * This method check whether this body can interact with other one or not. 
     * 
     * @return 
     *      Return indication whether two bodies can collide according 
     *      to the assigned category or not. 
     * 
     * @note
     *      The Box2d has implemented a similar approach. 
     */
    [[nodiscard]] bool CanInteract( const StaticBody* const other ) const noexcept {
        return  (m_categoryBits & other->m_collideWithBits) != 0 &&
                (m_collideWithBits & other->m_categoryBits) != 0;
    }

    /**
     * This method perform an intersection check for two bodies.
     * 
     * @return 
     *      Return true if both bodies are intersecting false otherwise. 
     */
    [[nodiscard]] bool Intersect( const StaticBody *const body ) const noexcept {
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
        return m_fixture.has_value();
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
     *      model pointed by "m_fixture->m_entity" pointer.   
     */
    template<class Callable>
    void InvokeCallback(Callable&& callback) noexcept {
        std::invoke(callback, m_fixture->m_entity);
    }

protected:

    MaskType        m_categoryBits { 0 };       // I am a ... .
    MaskType        m_collideWithBits { 0 };    // I collide with a ... .
    cocos2d::Rect   m_shape {};
   
    std::optional<Fixture> m_fixture { std::nullopt }; 
};

class KinematicBody final : 
    public StaticBody 
{   
public:
    friend class PhysicWorld;

    KinematicBody(const cocos2d::Vec2& position, 
        const cocos2d::Size& size,
        float moveSpeed = MOVE_SPEED,
        float jumpSpeed = JUMP_SPEED
    ) :
        StaticBody{ position, size },
        m_moveSpeed { moveSpeed },
        m_jumpSpeed { jumpSpeed },
        m_previousPosition { position }
    { 
    }
    
    /// Methods used by models (unit, projectile, etc)

    void SetXAxisSpeed(float speed) noexcept {
        m_moveSpeed = speed;
    }    

    void SetDirection(const cocos2d::Vec2& direction) noexcept {
        m_direction = direction;
    }

    cocos2d::Vec2 GetDirection() const noexcept {
        return m_direction;
    }

    /// Methods used by controllers ( input handler)

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

    [[nodiscard]] bool IsOnGround() const noexcept {
        return m_onGround;
    }

    /// Methods used by PhysicWorld in movement&collision implementation.
private:
    
    void MoveX(float dt) noexcept {
        m_previousPosition.x = m_shape.origin.x;
        m_shape.origin.x += m_direction.x * m_moveSpeed * dt;
    }

    void MoveY(float dt) noexcept {
        m_previousPosition.y = m_shape.origin.y;
        m_shape.origin.y += m_direction.y * dt * m_jumpSpeed;

        m_previousJumpTime = m_jumpTime;

        if( m_jumpTime > 0.f ) { 
            // jumping
            m_jumpTime -= dt;
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

    /// Properties
 private:   
    /**
     * This is x-axis speed of the body. 
     */
    float           m_moveSpeed { MOVE_SPEED };
    /**
     * This is y-axis speed of the body. 
     */
    float           m_jumpSpeed { JUMP_SPEED };
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

    /// TODO: decide whether it need to be modified in restoreY, ... .
    /**
     * Indicate whether a body is on ground or not. 
     */
    bool            m_onGround { false };

    /**
     * List of some constants. Need to be replaced later. 
     */
    static constexpr float JUMP_SPEED { 600.f };
    static constexpr float JUMP_TIME  { 0.55f };
    static constexpr float MOVE_SPEED { 550.f };
};

#endif // PHYSIC_BODY_HPP