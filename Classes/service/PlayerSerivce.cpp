#include "PlayerService.h"
#include "Player.h"
#include "service/MapService.h"
#include "GameMap.h"
using namespace cocos2d;

void PlayerService::init(Scene* scene, MapService* ms) {
    player = Player::create();
    scene->addChild(player, 1);
    if (ms && ms->getMap()) {
        Vec2 tilePos(14.5f, 15.0f);
        Vec2 worldPos = ms->getMap()->convertToWorldCoord(tilePos);
        player->setPosition(worldPos);
        player->setGameMap(ms->getMap());
    }
    player->initKeyboardListener();
    player->initMouseListener();
    player->setScale(3.0f);
}

void PlayerService::update(float dt) {
    if (player) player->update(dt);
}