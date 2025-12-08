#pragma once
#include "cocos2d.h"
#include "Player.h"

class MapService;

class PlayerService {
public:
    void init(cocos2d::Scene* scene, MapService* ms);
    void update(float dt);
    Player* getPlayer() const { return player; }

private:
Player* player = nullptr;
};