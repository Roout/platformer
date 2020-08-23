#ifndef RESTART_SCENE_HPP
#define RESTART_SCENE_HPP

#include "cocos2d.h"

class PauseNode : public cocos2d::Node {
public:
    static PauseNode* create(size_t id);

    bool init() override;

    ~PauseNode();
private:
    /**
     * @param id 
     *  Level's id you wish to restart.
     */
    PauseNode(size_t id);

    // id of the scene you with restart
    size_t m_id { 0 }; 
};

#endif // RESTART_SCENE_HPP