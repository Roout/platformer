#include "CCArmatureDisplay.h"
#include "CCSlot.h"

DRAGONBONES_NAMESPACE_BEGIN

CCArmatureDisplay* CCArmatureDisplay::create()
{
    CCArmatureDisplay* displayContainer = new (std::nothrow) CCArmatureDisplay();
    if (displayContainer && displayContainer->init())
    if (displayContainer)
    {
        displayContainer->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(displayContainer);
    }

    return displayContainer;
}

void CCArmatureDisplay::dbInit(Armature* armature)
{
    _armature = armature;
}

void CCArmatureDisplay::dbClear()
{
    setEventDispatcher(cocos2d::Director::getInstance()->getEventDispatcher());

    _armature = nullptr;
    CC_SAFE_RELEASE(_dispatcher);
    release();
}

void CCArmatureDisplay::dispose(bool disposeProxy)
{
    if (_armature != nullptr) 
    {
        _armature->dispose();
        _armature = nullptr;
    }
}

void CCArmatureDisplay::dbUpdate()
{
    const auto drawed = DragonBones::debugDraw;
    if (drawed || _debugDraw) 
    {
        _debugDraw = drawed;
        if (_debugDraw) 
        {

        }
        else 
        {
            // TODO
        }
    }
}

void CCArmatureDisplay::addDBEventListener(const std::string& type, const std::function<void(EventObject*)>& callback)
{
    auto lambda = [callback](cocos2d::EventCustom* event) -> void 
    {
        callback(static_cast<EventObject*>(event->getUserData()));
    };
    _dispatcher->addCustomEventListener(type, lambda);
}

void CCArmatureDisplay::dispatchDBEvent(const std::string& type, EventObject* value)
{
    _dispatcher->dispatchCustomEvent(type, value);
}

void CCArmatureDisplay::removeDBEventListener(const std::string& type, const std::function<void(EventObject*)>& callback)
{
    // TODO
    _dispatcher->removeCustomEventListeners(type);
}

cocos2d::Rect CCArmatureDisplay::getBoundingBox() const
{
    auto isFirst = true;
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    for (const auto slot : _armature->getSlots())
    {
        if (!slot->getVisible() || !slot->getDisplay())
        { 
            continue;
        }
        
        const auto display = static_cast<CCSlot*>(slot)->getCCDisplay();
        const auto bounds = display->getBoundingBox();
        if (isFirst)
        {
            isFirst = false;
            minX = bounds.getMinX();
            minY = bounds.getMinY();
            maxX = bounds.getMaxX();
            maxY = bounds.getMaxY();
        }
        else
        {
            minX = std::min(minX, bounds.getMinX());
            minY = std::min(minY, bounds.getMinY());
            maxX = std::max(maxX, bounds.getMaxX());
            maxY = std::max(maxY, bounds.getMaxY());
        }
    }

    cocos2d::Rect rect(minX, minY, maxX - minX, maxY - minY);

    return cocos2d::RectApplyTransform(rect, getNodeToParentTransform());
}

DBCCSprite* DBCCSprite::create()
{
    DBCCSprite* sprite = new (std::nothrow) DBCCSprite();

    if (sprite && sprite->init())
    {
        sprite->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(sprite);
    }

    return sprite;
}
/** \update
 * \lib cocos2dx 4.0
 * \brief change check visibility from approach of testing center point 
 * to testing for the whole rect intersection
 * \author Roout
 * \date 06.08.2020
 */
bool DBCCSprite::_checkVisibility(const cocos2d::Mat4& transform, const cocos2d::Size& size, const cocos2d::Rect& rect)
{
    const auto scene = cocos2d::Director::getInstance()->getRunningScene();

    //If draw to Rendertexture, return true directly.
    // only cull the default camera. The culling algorithm is valid for default camera.
    if (!scene || (scene && scene->getDefaultCamera() != cocos2d::Camera::getVisitingCamera()))
        return true;

    const auto director = cocos2d::Director::getInstance();
    cocos2d::Rect visibleRect(director->getVisibleOrigin(), director->getVisibleSize());

    // convert content size to world coordinates
    const auto wsw = std::max(
        fabsf(size.width * transform.m[0] + size.height * transform.m[4]), 
        fabsf(size.width * transform.m[0] - size.height * transform.m[4])
    );
    const auto wsh = std::max(
        fabsf(size.width * transform.m[1] + size.height * transform.m[5]), 
        fabsf(size.width * transform.m[1] - size.height * transform.m[5])
    );
    
    // current position in local coordinates
    auto pos3d { cocos2d::Vec3::ZERO };
    // current position in world coordinates
    transform.getTranslation(&pos3d);

    const auto pos2d = cocos2d::Camera::getVisitingCamera()->projectGL(pos3d);
    // object's boundaries
    cocos2d::Rect boundingBox { pos2d, { wsw, wsh } };

    // enlarge visible rect by @size in screen coord
    visibleRect.origin.x -= wsw;
    visibleRect.origin.y -= wsh;
    visibleRect.size.width += wsw * 2;
    visibleRect.size.height += wsh * 2;

    const auto ret { boundingBox.intersectsRect(visibleRect) };
    return ret;
}

void DBCCSprite::draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags)
{
#if CC_USE_CULLING
#if COCOS2D_VERSION >= 0x00031400
    const auto& rect = _polyInfo.getRect();
#else
    const auto& rect = _polyInfo.rect;
#endif
    
    // Don't do calculate the culling if the transform was not updated
    auto visitingCamera = cocos2d::Camera::getVisitingCamera();
    auto defaultCamera = cocos2d::Camera::getDefaultCamera();
    if (visitingCamera == defaultCamera) {
        _insideBounds = ((flags & FLAGS_TRANSFORM_DIRTY) || visitingCamera->isViewProjectionUpdated()) ? _checkVisibility(transform, _contentSize, rect) : _insideBounds;
    }
    else
    {
        _insideBounds = _checkVisibility(transform, _contentSize, rect);
    }

    if (_insideBounds)
#endif
    {
		// \update
		// \lib cocos2dx 4.0
		// \brief adjust passed parameters: _texture; remove deprecated parameter.
		// \author Roout
		// \date 24.03.2020
        _trianglesCommand.init(_globalZOrder, _texture, _blendFunc, _polyInfo.triangles, transform, flags);
        renderer->addCommand(&_trianglesCommand);

#if CC_SPRITE_DEBUG_DRAW
        _debugDrawNode->clear();
        auto count = _polyInfo.triangles.indexCount / 3;
        auto indices = _polyInfo.triangles.indices;
        auto verts = _polyInfo.triangles.verts;
        for (ssize_t i = 0; i < count; i++)
        {
            //draw 3 lines
            auto from = verts[indices[i * 3]].vertices;
            auto to = verts[indices[i * 3 + 1]].vertices;
            _debugDrawNode->drawLine(cocos2d::Vec2(from.x, from.y), cocos2d::Vec2(to.x, to.y), cocos2d::Color4F::WHITE);

            from = verts[indices[i * 3 + 1]].vertices;
            to = verts[indices[i * 3 + 2]].vertices;
            _debugDrawNode->drawLine(cocos2d::Vec2(from.x, from.y), cocos2d::Vec2(to.x, to.y), cocos2d::Color4F::WHITE);

            from = verts[indices[i * 3 + 2]].vertices;
            to = verts[indices[i * 3]].vertices;
            _debugDrawNode->drawLine(cocos2d::Vec2(from.x, from.y), cocos2d::Vec2(to.x, to.y), cocos2d::Color4F::WHITE);
        }
#endif //CC_SPRITE_DEBUG_DRAW
    }
}

cocos2d::PolygonInfo& DBCCSprite::getPolygonInfoModify()
{
    return _polyInfo;
}

DRAGONBONES_NAMESPACE_END
