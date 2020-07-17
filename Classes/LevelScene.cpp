#include "LevelScene.hpp"
#include "Unit.hpp"
#include "UnitView.hpp"
#include "UserInputHandler.hpp"
#include "TileMapParser.hpp"
#include "BarrelManager.hpp"
#include "SmoothFollower.hpp"
#include "HealthBar.hpp"
#include "PhysicsHelper.hpp"

cocos2d::Scene* LevelScene::createScene(int id) {
    auto scene { cocos2d::Scene::createWithPhysics() };
    auto world = scene->getPhysicsWorld();
    // set gravity
    world->setGravity(cocos2d::Vec2(0, -1000));

    // optional: set debug draw
    world->setDebugDrawMask(cocos2d::PhysicsWorld::DEBUGDRAW_ALL);

    auto level { LevelScene::create(id) };

    scene->addChild(level);

    return scene;
}

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
            shapes[BODY_A]->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR),
            shapes[BODY_B]->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR)
        };

        // There are nodes one of which is with hero sensor attached 
        // i.e. basicaly it's hero and other body
        if (nodes[BODY_A] && nodes[BODY_B] && (isHeroSensor[BODY_A] || isHeroSensor[BODY_B]) ) {
            HeroView * heroView { dynamic_cast<HeroView*>(isHeroSensor[BODY_A]? nodes[BODY_A] : nodes[BODY_B]) };
            heroView->SetContactWithGround(true);

            return true;
        } 
        
        const int bodyMasks[2] = {
            bodies[BODY_A]->getCategoryBitmask(),
            bodies[BODY_B]->getCategoryBitmask()
        };
        const bool isPlatform[2] = {
            bodyMasks[BODY_A] == core::CreateMask(core::CategoryBits::PLATFORM),
            bodyMasks[BODY_B] == core::CreateMask(core::CategoryBits::PLATFORM)
        };
        // contact of any unit's body and platform starts
        if( isPlatform[BODY_A] || isPlatform[BODY_B] ) {
            const auto platformIndex { isPlatform[BODY_A]? BODY_A: BODY_B };
            const auto moveUpwards { helper::IsGreater(bodies[platformIndex ^ 1]->getVelocity().y, 0.f, 0.000001f) };

            // Ordinates before collision:
            const auto unitBottomOrdinate { nodes[platformIndex ^ 1]->getPosition().y };
            const auto platformTopOrdinate { 
                nodes[platformIndex]->getPosition().y + 
                nodes[platformIndex]->getContentSize().height
            };
            const auto canPassThrough { helper::IsLesser(unitBottomOrdinate, platformTopOrdinate, 0.00001f) };
            
            return !(moveUpwards || canPassThrough);
        }

        // handle contact of spikes and unit
        const bool isTrap[2] = {
            bodyMasks[BODY_A] == core::CreateMask(core::CategoryBits::TRAP),
            bodyMasks[BODY_B] == core::CreateMask(core::CategoryBits::TRAP)
        };
        if( isTrap[BODY_A] || isTrap[BODY_B] ) {
            const auto trapIndex { isTrap[BODY_A]? BODY_A: BODY_B };

            auto unit { dynamic_cast<HeroView*>(nodes[trapIndex^1]) };
            unit->AddCurse<Curses::CurseType::DPS>(10.f, Curses::UNLIMITED);
            return true;
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
            shapes[BODY_A]->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR),
            shapes[BODY_B]->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR)
        };

        if (nodes[BODY_A] && nodes[BODY_B] && (isHeroSensor[BODY_A] || isHeroSensor[BODY_B]) ) {
            HeroView * heroView { dynamic_cast<HeroView*>(isHeroSensor[BODY_A]? nodes[BODY_A] : nodes[BODY_B]) };
            bool onGround {
                isHeroSensor[BODY_A]? 
                    helper::IsEquel(bodies[BODY_A]->getVelocity().y, 0.f, 0.000001f):
                    helper::IsEquel(bodies[BODY_B]->getVelocity().y, 0.f, 0.000001f)
            };
            heroView->SetContactWithGround(onGround);
            return true;
        }

        // handle contact of spikes and unit
        const bool isTrap[2] = {
            bodyMasks[BODY_A] == core::CreateMask(core::CategoryBits::TRAP),
            bodyMasks[BODY_B] == core::CreateMask(core::CategoryBits::TRAP)
        };
        if( isTrap[BODY_A] || isTrap[BODY_B] ) {
            const auto trapIndex { isTrap[BODY_A]? BODY_A: BODY_B };

            auto unit { dynamic_cast<HeroView*>(nodes[trapIndex^1]) };
            unit->RemoveCurse<Curses::CurseType::DPS>();
            return true;
        }


        return true;
    }

};

void LevelScene::InitTileMapObjects(cocos2d::FastTMXTiledMap * map) {
    TileMapParser parser{ map };
    parser.Parse();

    m_borders.reserve(5000);
    m_platforms.reserve(200);
    m_barrelManager = std::make_unique<BarrelManager>();

    for(size_t i = 0; i < core::EnumSize<core::CategoryName>(); i++) {
        const auto category { static_cast<core::CategoryName>(i) };
        auto parsedForms { parser.Acquire(category) };
        for(const auto& form: parsedForms) {
            if ( form.m_type == core::CategoryName::PLAYER ) {
                m_playerPosition = form.m_botLeft;
            } 
            else if(form.m_type == core::CategoryName::PLATFORM ) {
                auto body = cocos2d::PhysicsBody::createBox(form.m_rect.size);
                body->setDynamic(false);
                
                auto node = Node::create();
                node->setContentSize(form.m_rect.size);
                node->setPosition(form.m_rect.origin);
                node->addComponent(body);

                map->addChild(node);
                m_platforms.emplace_back(body);
            }
            else if(form.m_type == core::CategoryName::BORDER) {
                auto body = cocos2d::PhysicsBody::createBox(form.m_rect.size);
                body->setDynamic(false);
                body->setPositionOffset(form.m_rect.size / 2.f);
                
                auto node = Node::create();
                node->setPosition(form.m_rect.origin);
                node->addComponent(body);

                map->addChild(node);
                m_borders.emplace_back(body);
            }
            else if(form.m_type == core::CategoryName::SPIKES) {
                auto body = cocos2d::PhysicsBody::createBox(form.m_rect.size);
                body->setDynamic(false);
                
                auto node = Node::create();
                node->setContentSize(form.m_rect.size);
                node->setPosition(form.m_rect.origin);
                node->addComponent(body);

                map->addChild(node);
                m_traps.emplace_back(body);
            }
            else if(form.m_type == core::CategoryName::BARREL) {
                auto barrel { std::make_unique<Barrel>() };
                auto barrelView { BarrelView::create(barrel.get()) };
                
                barrelView->setPosition(form.m_botLeft);
                barrel->AddPhysicsBody(barrelView->getPhysicsBody());

                map->addChild(barrelView);
                m_barrelManager->Add(move(barrel), barrelView);
            }
        }
    }
}

bool LevelScene::init() {
	if (!cocos2d::Scene::init()) {
		return false;
	}
	this->scheduleUpdate();

	const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
	const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
    
    const auto tmxFile { cocos2d::StringUtils::format("Map/level_%d.tmx", m_id) };
    cocos2d::FastTMXTiledMap *tileMap { cocos2d::FastTMXTiledMap::create(tmxFile) };
    tileMap->setName("Map");
    this->addChild(tileMap);

    this->InitTileMapObjects(tileMap);
   
    m_unit = std::make_shared<Unit>();
    auto playerNode { HeroView::create(m_unit.get()) };
    const auto body { playerNode->getPhysicsBody() };
    const auto unitBodySize { m_unit->GetSize() };

    m_unit->AddBody(body);
    /// TODO: MUST be initialized after attaching body to unit! This looks bad as design!
    m_movement      = std::make_unique<Movement> (m_unit.get());
    m_inputHandler  = std::make_unique<UserInputHandler>(m_unit.get(), m_movement.get(), this);

    playerNode->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    playerNode->setPosition(m_playerPosition);
    playerNode->setName("Player");
    tileMap->addChild(playerNode, 10);

    const auto mapShift { m_playerPosition 
        - cocos2d::Vec2{ visibleSize.width / 2.f, visibleSize.height / 3.f } 
        - origin 
    };
    tileMap->setPosition(-mapShift);

    /// TODO: Get rid of the requirement to have Node* being created with new position!
    m_playerFollower = std::make_unique<SmoothFollower>(m_unit);

    /// TODO: move somewhere
    static constexpr float healthBarShift { 15.f };
    HealthBar *bar = HealthBar::create(m_unit);
    bar->setPosition(-unitBodySize.width / 2.f, unitBodySize.height + healthBarShift);
    playerNode->addChild(bar);

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
   
    m_movement->Update(dt);

    m_unit->UpdateWeapon(dt);
    m_unit->UpdateState(dt);
    
    m_playerFollower->UpdateAfterUnitMove(dt);
    
    auto mapNode = this->getChildByName("Map");
    m_playerFollower->UpdateNodePosition(mapNode);

    m_barrelManager->Update();

    m_unit->UpdateCurses(dt);

    static unsigned int x { 0 };
    cocos2d::log("Update: %0.4f, %d", dt, x++);
    // const auto position { ->getPosition() };
    // cocos2d::log("Unit is at: [%0.4f, %0.4f]", position.x, position.y );
}
