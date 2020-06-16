#ifndef BARREL_MANAGER_HPP
#define BARREL_MANAGER_HPP

#include "Barrel.hpp"
#include "BarrelView.hpp"
#include "cocos2d.h"

#include <vector>
#include <memory>
#include <algorithm>

/**
 *  It will remove already exploded barrels and give some time 
 * to barrel view to play explosion animation. Then remove it too.
 */
class BarrelManager final {
public:

    BarrelManager() {
        m_barrels.reserve(15);
        m_explosions.reserve(15);
    } 

    /**
     * TODO: improve algorithm
     * 
     * - Remove all just exploded models of the barrel.
     * - Remove all views that had already played their explosion animation.
     */
    void Update() noexcept {
        // remove exploded barrels
        for(auto& [barrel, view]: m_barrels) {
            if( barrel->IsExploded() ) {
                view->Explode();
                m_explosions.emplace_back(view);

                // remove pair
                barrel.reset();
                view = nullptr;
            } 
        }

        m_barrels.erase(
            std::remove_if(m_barrels.begin(), m_barrels.end(), [](const auto& pair) {
                return (pair.second == nullptr);
            }),
            m_barrels.end()
        );

        // remove finished animations
        for(auto& explosion: m_explosions) {
            if(explosion->Finished()) {
                /// TODO: check for parent before removing it
                explosion->removeFromParent();
                explosion = nullptr;
            }
        }

        m_explosions.erase(
            std::remove(m_explosions.begin(), m_explosions.end(), nullptr),
            m_explosions.end()
        );
    }

    /**
     * Add barrel to manager
     * 
     * @param[in] barrel
     *      Barrel model created via std::make_unique<>
     * @param[in] view 
     *      Need to be retained by adding it to some parent node.
     */
    void Add(std::unique_ptr<Barrel>&& barrel, BarrelView* view) {
        m_barrels.emplace_back(std::move(barrel), view);
    }

private:

    std::vector<std::pair<std::unique_ptr<Barrel>, BarrelView*>> m_barrels;
    /**
     * Keep already exploded barrel views. 
     */
    std::vector<BarrelView*> m_explosions;
};


#endif // BARREL_MANAGER_HPP