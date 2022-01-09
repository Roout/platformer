#include "ContactHandler.hpp"
#include "Utils.hpp"
#include "Core.hpp"
#include "PhysicsHelper.hpp"

#include "components/Traps.hpp"
#include "components/Props.hpp"
#include "components/Projectile.hpp"
#include "components/CurseHub.hpp"

#include "units/Unit.hpp"
#include "units/Bot.hpp"
#include "units/Player.hpp"

#include "cocos2d.h"

namespace contact {

bool OnContactBegin(cocos2d::PhysicsContact& contact) {
    enum { BODY_A, BODY_B };

    cocos2d::PhysicsShape * const shapes[2] = { 
        contact.getShapeA(),
        contact.getShapeB() 
    };
    cocos2d::PhysicsBody * const bodies[2] = { 
        shapes[BODY_A]->getBody(),
        shapes[BODY_B]->getBody()
    };
    cocos2d::Node * const nodes[2] = { 
        bodies[BODY_A]->getNode(),
        bodies[BODY_B]->getNode() 
    };
    
    if(!nodes[BODY_A] || !nodes[BODY_B]) {
        return false;
    }

    const bool isUnitSensor[2] = { 
        shapes[BODY_A]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::GROUND_SENSOR),
        shapes[BODY_B]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::GROUND_SENSOR)
    };

    // There are nodes one of which is with unit sensor attached 
    // i.e. basicaly it's unit and other collidable body
    if (isUnitSensor[BODY_A] || isUnitSensor[BODY_B]) {
        Unit * unit { static_cast<Unit*>(isUnitSensor[BODY_A]? nodes[BODY_A] : nodes[BODY_B]) };
        unit->EnableContactWithGround();

        return true;
    } 
    
    /// Platform & Unit
    const int bodyMasks[2] = {
        bodies[BODY_A]->getCategoryBitmask(),
        bodies[BODY_B]->getCategoryBitmask()
    };

    const bool isPlatform[2] = {
        bodyMasks[BODY_A] == Utils::CreateMask(core::CategoryBits::PLATFORM),
        bodyMasks[BODY_B] == Utils::CreateMask(core::CategoryBits::PLATFORM)
    };

    const auto unitMask { 
        Utils::CreateMask(core::CategoryBits::PLAYER, core::CategoryBits::ENEMY) 
    };
    const bool isUnit[2] = {
        (bodyMasks[BODY_A] & unitMask) > 0,
        (bodyMasks[BODY_B] & unitMask) > 0
    };

    if((isPlatform[BODY_A] || isPlatform[BODY_B]) && (isUnit[BODY_A] || isUnit[BODY_B])) {
        const auto platformIndex { isPlatform[BODY_A]? BODY_A: BODY_B };
        const auto moveUpwards { helper::IsGreater(bodies[platformIndex ^ 1]->getVelocity().y, 0.f, 0.000001f) };

        // Ordinates before collision:
        const auto unitBottomOrdinate { nodes[platformIndex ^ 1]->getPosition().y };
        const auto platformTopOrdinate { 
            nodes[platformIndex]->getPosition().y + 
            nodes[platformIndex]->getContentSize().height / 2.f
        };
        const auto canPassThrough { helper::IsLesser(unitBottomOrdinate, platformTopOrdinate, 0.00001f) };
        
        return !(moveUpwards || canPassThrough);
    }

    /// Spikes & Unit
    const bool isTrap[2] = {
        bodyMasks[BODY_A] == Utils::CreateMask(core::CategoryBits::TRAP),
        bodyMasks[BODY_B] == Utils::CreateMask(core::CategoryBits::TRAP)
    };
    if(isTrap[BODY_A] || isTrap[BODY_B]) {
        const auto trapIndex { isTrap[BODY_A]? BODY_A: BODY_B };

        const auto unit { static_cast<Unit*>(nodes[trapIndex^1]) };
        const auto trap { static_cast<traps::Trap*>(nodes[trapIndex]) };
        
        trap->CurseTarget(unit);

        return false;
    }

    /// Unit & Unit
    if(isUnit[BODY_A] && isUnit[BODY_B]) {
        const auto enemyMask { Utils::CreateMask(core::CategoryBits::ENEMY) };
        const auto playerIndex { (bodyMasks[BODY_A] & enemyMask) ? BODY_B: BODY_A };
        const auto enemiesCount { 
            ((bodyMasks[BODY_A] & enemyMask) > 0) 
            + ((bodyMasks[BODY_B] & enemyMask) > 0) 
        };
        if (enemiesCount == 1) {
            const auto player { static_cast<Unit*>(nodes[playerIndex]) };
            const auto enemy { static_cast<Enemies::Bot*>(nodes[playerIndex^1]) };
            player->AddCurse<curses::CurseClass::DPS>(enemy->GetId(), Player::DAMAGE_ON_CONTACT, curses::UNLIMITED);
        }
        return false;
    }

    /// Projectile & (Unit or Barrel)
    const auto projectileMask { 
        Utils::CreateMask(core::CategoryBits::PLAYER_PROJECTILE, core::CategoryBits::ENEMY_PROJECTILE)
    };
    const bool isProjectile[2] = {
        (bodyMasks[BODY_A] & projectileMask) > 0,
        (bodyMasks[BODY_B] & projectileMask) > 0
    };

    if(isProjectile[BODY_A] && isProjectile[BODY_B]) {
        const auto projA { static_cast<Projectile*>(nodes[BODY_A]) };
        const auto projB { static_cast<Projectile*>(nodes[BODY_B]) };
        projA->Collapse();
        projB->Collapse();
        return false;
    }
    else if(isProjectile[BODY_A] || isProjectile[BODY_B]) {
        const auto projectileIndex { isProjectile[BODY_A]? BODY_A: BODY_B };
        const auto proj { static_cast<Projectile*>(nodes[projectileIndex]) };
        proj->SetExplosionState(Projectile::State::HIT_GROUND);
        
        // damage target if possible
        if(isUnit[projectileIndex ^ 1]) {
            const auto unit { static_cast<Unit*>(nodes[projectileIndex ^ 1]) };
            unit->AddCurse<curses::CurseClass::INSTANT>(curses::CurseHub::ignored, proj->GetDamage());
            proj->SetExplosionState(Projectile::State::HIT_PLAYER);
        } 
        else if(bodyMasks[projectileIndex ^ 1] == Utils::CreateMask(core::CategoryBits::PROPS)) {
            const auto prop { static_cast<props::Prop*>(nodes[projectileIndex^1]) };
            prop->Explode();
        }

        // destroy projectile
        proj->Collapse();
        // end contact, no need to process collision
        return false;
    }

    return true;
}

bool OnContactSeparate(cocos2d::PhysicsContact& contact) {
    enum { BODY_A, BODY_B };

    cocos2d::PhysicsShape * const shapes[2] = { 
        contact.getShapeA(),
        contact.getShapeB() 
    };
    cocos2d::PhysicsBody * const bodies[2] = { 
        shapes[BODY_A]->getBody(),
        shapes[BODY_B]->getBody()
    };
    cocos2d::Node * const nodes[2] = { 
        bodies[BODY_A]->getNode(),
        bodies[BODY_B]->getNode() 
    };

    if( !nodes[BODY_A] || !nodes[BODY_B] ) {
        return false;
    }

    const int bodyMasks[2] = {
        bodies[BODY_A]->getCategoryBitmask(),
        bodies[BODY_B]->getCategoryBitmask()
    };

    bool isUnitSensor[2] = { 
        shapes[BODY_A]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::GROUND_SENSOR),
        shapes[BODY_B]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::GROUND_SENSOR)
    };

    if (nodes[BODY_A] && nodes[BODY_B] && (isUnitSensor[BODY_A] || isUnitSensor[BODY_B]) ) {
        const auto unit { static_cast<Unit*>(isUnitSensor[BODY_A]? nodes[BODY_A] : nodes[BODY_B]) };
        bool onGround {
            isUnitSensor[BODY_A]? 
                helper::IsEqual(bodies[BODY_A]->getVelocity().y, 0.f, 0.000001f):
                helper::IsEqual(bodies[BODY_B]->getVelocity().y, 0.f, 0.000001f)
        };
        if (onGround) {
            unit->EnableContactWithGround();
        }
        else {
            unit->DisableContactWithGround();
        }
        return true;
    }

    // handle contact of spikes and unit
    const bool isTrap[2] = {
        bodyMasks[BODY_A] == Utils::CreateMask(core::CategoryBits::TRAP),
        bodyMasks[BODY_B] == Utils::CreateMask(core::CategoryBits::TRAP)
    };
    if( isTrap[BODY_A] || isTrap[BODY_B] ) {
        const auto trapIndex { isTrap[BODY_A]? BODY_A: BODY_B };

        const auto unit { static_cast<Unit*>(nodes[trapIndex^1]) };
        const auto trap { static_cast<traps::Trap*>(nodes[trapIndex]) };
        
        trap->RemoveCurse(unit);

        return false;
    }

    /// Unit & Unit
    const auto unitMask { Utils::CreateMask(core::CategoryBits::PLAYER, core::CategoryBits::ENEMY) };
    const bool isUnit[2] = {
        (bodyMasks[BODY_A] & unitMask) > 0,
        (bodyMasks[BODY_B] & unitMask) > 0
    };
    if(isUnit[BODY_A] && isUnit[BODY_B]) {
        const auto enemyMask { Utils::CreateMask(core::CategoryBits::ENEMY) };
        const auto playerIndex { (bodyMasks[BODY_A] & enemyMask) > 0 ? BODY_B: BODY_A };
        const auto enemiesCount { 
            ((bodyMasks[BODY_A] & enemyMask) > 0) 
            + ((bodyMasks[BODY_B] & enemyMask) > 0) 
        };
        if (enemiesCount == 1) {
            const auto player { static_cast<Unit*>(nodes[playerIndex]) };
            const auto enemy { static_cast<Enemies::Bot*>(nodes[playerIndex^1]) };
            player->RemoveCurse(enemy->GetId());
        }
        return false;
    }


    return true;
}

} // namespace contact