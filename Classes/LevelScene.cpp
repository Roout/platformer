#include "LevelScene.hpp"
#include "Unit.hpp"
#include "Barrel.hpp"
#include "Border.hpp"
#include "Platform.hpp"
#include "UserInputHandler.hpp"
#include "TileMapParser.hpp"
#include "SmoothFollower.hpp"
#include "HealthBar.hpp"
#include "PhysicsHelper.hpp"
#include "Utils.hpp"
#include "Projectile.hpp"
#include "Traps.hpp"
#include "SizeDeducer.hpp"

LevelScene::LevelScene(int id): 
    m_id{ id } 
{
}

LevelScene* LevelScene::create(int id) {
    auto *pRet = new(std::nothrow) LevelScene{id};
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
} 

namespace helper {
    
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
        
        const bool isHeroSensor[2] = { 
            shapes[BODY_A]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::HERO_SENSOR),
            shapes[BODY_B]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::HERO_SENSOR)
        };

        // There are nodes one of which is with hero sensor attached 
        // i.e. basicaly it's hero and other body
        if (nodes[BODY_A] && nodes[BODY_B] && (isHeroSensor[BODY_A] || isHeroSensor[BODY_B]) ) {
            Unit * heroView { dynamic_cast<Unit*>(isHeroSensor[BODY_A]? nodes[BODY_A] : nodes[BODY_B]) };
            heroView->HasContactWithGround(true);

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
        if( isPlatform[BODY_A] || isPlatform[BODY_B] ) {
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
        if( isTrap[BODY_A] || isTrap[BODY_B] ) {
            const auto trapIndex { isTrap[BODY_A]? BODY_A: BODY_B };

            const auto unit { dynamic_cast<Unit*>(nodes[trapIndex^1]) };
            const auto trap { dynamic_cast<Traps::Trap*>(nodes[trapIndex]) };
            
            trap->CurseTarget(unit);

            return false;
        }

        /// Projectile & (Unit or Barrel)
        const bool isProjectile[2] = {
            bodyMasks[BODY_A] == Utils::CreateMask(core::CategoryBits::PROJECTILE),
            bodyMasks[BODY_B] == Utils::CreateMask(core::CategoryBits::PROJECTILE)
        };
        const bool isUnit[2] = {
            bodyMasks[BODY_A] == Utils::CreateMask(core::CategoryBits::HERO, core::CategoryBits::ENEMY),
            bodyMasks[BODY_B] == Utils::CreateMask(core::CategoryBits::HERO, core::CategoryBits::ENEMY)
        };
        if( isProjectile[BODY_A] || isProjectile[BODY_B] ) {
            const auto projectileIndex { isProjectile[BODY_A]? BODY_A: BODY_B };

            auto proj { dynamic_cast<Projectile*>(nodes[projectileIndex]) };
            
            // damage target if possible
            if(isUnit[projectileIndex ^ 1]) {
                const auto unit { dynamic_cast<Unit*>(nodes[projectileIndex^1]) };
                unit->AddCurse<Curses::CurseType::INSTANT>(
                    Curses::CurseHub::ignored, 
                    proj->GetDamage()
                );
            } else if( bodyMasks[projectileIndex ^ 1] == Utils::CreateMask(core::CategoryBits::BARREL)) {
                const auto barrel { dynamic_cast<Barrel*>(nodes[projectileIndex^1]) };
                barrel->Explode();
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

        const int bodyMasks[2] = {
            bodies[BODY_A]->getCategoryBitmask(),
            bodies[BODY_B]->getCategoryBitmask()
        };

        bool isHeroSensor[2] = { 
            shapes[BODY_A]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::HERO_SENSOR),
            shapes[BODY_B]->getCategoryBitmask() == Utils::CreateMask(core::CategoryBits::HERO_SENSOR)
        };

        if (nodes[BODY_A] && nodes[BODY_B] && (isHeroSensor[BODY_A] || isHeroSensor[BODY_B]) ) {
            Unit * heroView { dynamic_cast<Unit*>(isHeroSensor[BODY_A]? nodes[BODY_A] : nodes[BODY_B]) };
            bool onGround {
                isHeroSensor[BODY_A]? 
                    helper::IsEquel(bodies[BODY_A]->getVelocity().y, 0.f, 0.000001f):
                    helper::IsEquel(bodies[BODY_B]->getVelocity().y, 0.f, 0.000001f)
            };
            heroView->HasContactWithGround(onGround);
            return true;
        }

        // handle contact of spikes and unit
        const bool isTrap[2] = {
            bodyMasks[BODY_A] == Utils::CreateMask(core::CategoryBits::TRAP),
            bodyMasks[BODY_B] == Utils::CreateMask(core::CategoryBits::TRAP)
        };
        if( isTrap[BODY_A] || isTrap[BODY_B] ) {
            const auto trapIndex { isTrap[BODY_A]? BODY_A: BODY_B };

            const auto unit { dynamic_cast<Unit*>(nodes[trapIndex^1]) };
            const auto trap { dynamic_cast<Traps::Trap*>(nodes[trapIndex]) };
            
            trap->RemoveCurse(unit);

            return false;
        }


        return true;
    }

};

void LevelScene::InitTileMapObjects(cocos2d::FastTMXTiledMap * map) {
    TileMapParser parser{ map };
    parser.Parse();

    for(size_t i = 0; i < Utils::EnumSize<core::CategoryName>(); i++) {
        const auto category { static_cast<core::CategoryName>(i) };
        const auto parsedForms { parser.Acquire(category) };
        for(const auto& form: parsedForms) {
            if ( form.m_type == core::CategoryName::PLAYER ) {
                const cocos2d::Size size { 
                    SizeDeducer::GetInstance().GetAdjustedSize(80.f), 
                    SizeDeducer::GetInstance().GetAdjustedSize(135.f)
                };
                const auto hero { Player::create(size) };
                hero->setName("Player");
                hero->setPosition(form.m_botLeft + size / 2.f);
                map->addChild(hero, 10);
            } 
            else if(form.m_type == core::CategoryName::PLATFORM ) {
                const auto platform = Platform::create(form.m_rect.size);
                platform->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(platform);
            }
            else if(form.m_type == core::CategoryName::BORDER) {
                const auto border { Border::create(form.m_rect.size) };
                border->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(border);
            }
            else if(form.m_type == core::CategoryName::SPIKES) {
                const auto trap = Traps::Spikes::create(form.m_rect.size);
                trap->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(trap);
            }
            else if(form.m_type == core::CategoryName::BARREL) {
                const auto barrel { Barrel::create() };
                barrel->setPosition(form.m_botLeft);
                map->addChild(barrel);
            }
        }
    }
}

bool LevelScene::init() {
	if (!cocos2d::Scene::initWithPhysics()) {
		return false;
	}

    auto world = this->getPhysicsWorld();
    world->setGravity(cocos2d::Vec2(0, -1000));
    world->setDebugDrawMask(cocos2d::PhysicsWorld::DEBUGDRAW_ALL);

	this->scheduleUpdate();
    
    const auto tmxFile { cocos2d::StringUtils::format("Map/level_%d.tmx", m_id) };
    cocos2d::FastTMXTiledMap *tileMap { cocos2d::FastTMXTiledMap::create(tmxFile) };
    tileMap->setName("Map");
    this->addChild(tileMap);
    this->InitTileMapObjects(tileMap);
   
    /// TODO: MUST be initialized after attaching body to unit! This looks bad as design!
    const auto player { dynamic_cast<Player*>(tileMap->getChildByName("Player"))};
    m_inputHandler  = std::make_unique<UserInputHandler>(player);

    const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
	const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
    const auto mapShift { player->getPosition() 
        - cocos2d::Vec2{ visibleSize.width / 2.f, visibleSize.height / 3.f } 
        - origin 
    };
    tileMap->setPosition(-mapShift);

    /// TODO: move somewhere
    static constexpr float healthBarShift { 15.f };
    HealthBar *bar = HealthBar::create(player);
    bar->setPosition(-player->getContentSize().width / 2.f, player->getContentSize().height + healthBarShift);
    player->addChild(bar);

    // Add physics body contact listener
    auto shapeContactListener = cocos2d::EventListenerPhysicsContact::create();
    shapeContactListener->onContactBegin = helper::OnContactBegin;
    shapeContactListener->onContactSeparate = helper::OnContactSeparate;
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(shapeContactListener, this);

    return true;
}

void LevelScene::menuCloseCallback(cocos2d::Ref* pSender) {
    //Close the cocos2d-x game scene and quit the application
    cocos2d::Director::getInstance()->end();
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void LevelScene::update(float dt) {

    // static unsigned int x { 0 };
    // cocos2d::log("Update: %0.4f, %d", dt, x++);
}
