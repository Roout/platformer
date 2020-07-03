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
    
    TileMapParser parser{ tileMap };
    parser.Parse();

    const auto playerPosition { parser.Acquire<ParsedType::PLAYER>() };
    const auto obstacles { parser.Acquire<ParsedType::STATIC_BODIES>()}; 

    m_borders.reserve(5000);
    m_platforms.reserve(200);

    m_barrelManager = std::make_unique<BarrelManager>();

    for(const auto& [shape, category] : obstacles ) {
        if (category == core::CategoryName::PLATFORM ) {
            auto body = cocos2d::PhysicsBody::createBox(shape.size);
            body->setPositionOffset(shape.size / 2.f);
            auto node = Node::create();
            node->setPosition(shape.origin);
            node->addComponent(body);
            tileMap->addChild(node);
            m_platforms.emplace_back(body);
        } 
        else if(category == core::CategoryName::BORDER) {
            auto body = cocos2d::PhysicsBody::createBox(shape.size);
            body->setPositionOffset(shape.size / 2.f);
            auto node = Node::create();
            node->setPosition(shape.origin);
            node->addComponent(body);
            tileMap->addChild(node);

            m_borders.emplace_back(body);
        }
        else if(category == core::CategoryName::BARREL) {
            /// TODO: move width and height either to Barrel model either to map as object info.
            const auto size { cocos2d::Size{ 80, 100 } };
            auto body = cocos2d::PhysicsBody::createBox(size);
            body->setPositionOffset(size / 2.f);

            auto barrel { std::make_unique<Barrel>(body,  size) };
            auto barrelView = BarrelView::create(barrel.get());
            barrelView->setPosition(shape.origin);
            barrelView->addComponent(body);
            tileMap->addChild(barrelView);
            m_barrelManager->Add(move(barrel), barrelView);
        }
    }
    m_unit              = std::make_shared<Unit>();

    auto playerNode { HeroView::create(m_unit.get()) };
    const auto body { playerNode->getPhysicsBody() };
    const auto unitBodySize { m_unit->GetSize() };

    m_unit->AddBody(body);
    /// TODO: MUST be initialized after attaching body to unit! This looks bad as design!
    m_movement      = std::make_unique<Movement> (m_unit.get());
    m_inputHandler  = std::make_unique<UserInputHandler>(m_unit.get(), m_movement.get(), this);

    playerNode->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_BOTTOM);
    playerNode->setPosition(playerPosition);
    playerNode->setName("Player");
    tileMap->addChild(playerNode, 10);

    const auto mapShift { playerPosition 
        - cocos2d::Vec2{ visibleSize.width / 2.f, visibleSize.height / 3.f } 
        - origin 
    };
    tileMap->setPosition(-mapShift);

    /// TODO: Get rid of the requirement to have Node* being created with new position!
    m_playerFollower    = std::make_unique<SmoothFollower>(m_unit);

    /// TODO: move somewhere
    static constexpr float healthBarShift { 15.f };
    HealthBar *bar = HealthBar::create(m_unit);
    bar->setPosition(-unitBodySize.width / 2.f, unitBodySize.height + healthBarShift);
    playerNode->addChild(bar);

    auto shapeContactListener = cocos2d::EventListenerPhysicsContact::create();
    shapeContactListener->onContactBegin = [](cocos2d::PhysicsContact& contact) {
        const auto shapeA { contact.getShapeA() };
        const auto shapeB { contact.getShapeB() };
        
        const auto bodyA { shapeA->getBody() };
        const auto bodyB { shapeB->getBody() };

        auto nodeA { bodyA->getNode() };
        auto nodeB { bodyB->getNode() };

        enum { BODY_A, BODY_B };
        bool isHeroSensor[2] = { 
            shapeA->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR),
            shapeB->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR)
        };

        if (nodeA && nodeB && (isHeroSensor[BODY_A] || isHeroSensor[BODY_B]) ) {
            HeroView * heroView { dynamic_cast<HeroView*>(isHeroSensor[BODY_A]? nodeA : nodeB) };
            // bool onGround {
            //     isHeroSensor[BODY_A]? 
            //         helper::IsEquel(bodyA->getVelocity().y, 0.f, 0.000001f):
            //         helper::IsEquel(bodyB->getVelocity().y, 0.f, 0.000001f)
            // };
            heroView->SetContactWithGround(true);
        }

        return true;
    };

    shapeContactListener->onContactSeparate = [](cocos2d::PhysicsContact& contact) {
        const auto shapeA { contact.getShapeA() };
        const auto shapeB { contact.getShapeB() };

        const auto bodyA { shapeA->getBody() };
        const auto bodyB { shapeB->getBody() };

        auto nodeA { bodyA->getNode() };
        auto nodeB { bodyB->getNode() };

        enum { BODY_A, BODY_B };
        bool isHeroSensor[2] = { 
            shapeA->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR),
            shapeB->getCategoryBitmask() == core::CreateMask(core::CategoryBits::HERO_SENSOR)
        };

        if (nodeA && nodeB && (isHeroSensor[BODY_A] || isHeroSensor[BODY_B]) ) {
            HeroView * heroView { dynamic_cast<HeroView*>(isHeroSensor[BODY_A]? nodeA : nodeB) };
            bool onGround {
                isHeroSensor[BODY_A]? 
                    helper::IsEquel(bodyA->getVelocity().y, 0.f, 0.000001f):
                    helper::IsEquel(bodyB->getVelocity().y, 0.f, 0.000001f)
            };
            heroView->SetContactWithGround(onGround);
        }

        return true;
    };

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
    
    auto director { cocos2d::Director::getInstance() };
    const auto visibleSize = director->getVisibleSize();
	const auto origin = director->getVisibleOrigin();
    auto mapNode = this->getChildByName("Map");
    m_playerFollower->UpdateNodePosition(mapNode);

    m_barrelManager->Update();

    static unsigned int x { 0 };
    cocos2d::log("Update: %0.4f, %d", dt, x++);
    // const auto position { ->getPosition() };
    // cocos2d::log("Unit is at: [%0.4f, %0.4f]", position.x, position.y );
}
