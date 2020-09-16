#ifndef CONTACT_HANDLER_HPP
#define CONTACT_HANDLER_HPP

namespace cocos2d {
    class PhysicsContact;
}

namespace contact {

    bool OnContactBegin(cocos2d::PhysicsContact& contact);

    bool OnContactSeparate(cocos2d::PhysicsContact& contact);

} // namespace contact

#endif // CONTACT_HANDLER_HPP