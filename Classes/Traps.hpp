#ifndef TRAPS_HPP
#define TRAPS_HPP

#include "cocos2d.h"
#include "Core.hpp"
#include "Utils.hpp"
#include "Curses.hpp"

#include <limits>

namespace Traps {

    /**
     * Do damage every m_maxCooldown seconds to each body it can influence
     */
    class Trap : public cocos2d::Node {
    public:

        size_t GetId() const noexcept {
            return m_id;
        }

        virtual void CurseTarget(Unit * const unit) = 0;
        
        void RemoveCurse(Unit* const unit) {
            unit->RemoveCurse(this->GetId());
        };

    protected:
        Trap( const cocos2d::Size& size ) :
            m_id { m_generator.Next() } 
        {   
            auto body { cocos2d::PhysicsBody::createBox(size)  };
            body->setDynamic(false);
            body->setCategoryBitmask(
                Utils::CreateMask(
                    core::CategoryBits::TRAP
                )
            );
            body->setContactTestBitmask(
                Utils::CreateMask(
                    core::CategoryBits::ENEMY, 
                    core::CategoryBits::HERO
                )
            );
            // Node part
            this->addComponent(body);
            this->setContentSize(size);
        }

    private:
        const size_t m_id { 0 };

        /// TODO: parse traps from Tiled as special object. This will provide a unique ID
        inline static Utils::LinearGenerator m_generator { 
            std::numeric_limits<size_t>::max() >> 2 
        };
    };

    class Spikes : public Trap {
    public:
        static Spikes* create(const cocos2d::Size & size) {
            auto pRet = new (std::nothrow) Spikes(size);
            if (pRet && pRet->init()) {
                pRet->autorelease();
            }
            else {
                delete pRet;
                pRet = nullptr;
            }
            return pRet;
        }

        void CurseTarget(Unit * const unit) override {
            unit->AddCurse<Curses::CurseClass::DPS>(this->GetId(), m_damage, Curses::UNLIMITED);
        };

    private:
        Spikes(const cocos2d::Size & size) : 
            Trap { size } {}

        static constexpr float m_damage { 10.f };
    };
}

#endif // TRAPS_HPP