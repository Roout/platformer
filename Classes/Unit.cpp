#include "Unit.h"
#include "PhysicWorld.h"
#include "cocos2d.h"


Unit::Unit(PhysicWorld *world, float x, float y): 
    m_physicWorld{ world },
    m_body { m_physicWorld->Create<KinematicBody>( { x, y }, { m_width, m_height })}
{
    m_body->SetMask(
        CreateMask(CategoryBits::HERO),
        CreateMask(CategoryBits::ENEMY, CategoryBits::BOUNDARY) 
    );
}

Unit::~Unit() {
    m_physicWorld->Erase<KinematicBody>(m_body);
}
