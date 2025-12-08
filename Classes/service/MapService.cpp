#include "MapService.h"
#include "GameMap.h"
#include "Player.h"
#include "service/PlayerService.h"
#include "service/AudioService.h"
#include "service/UIService.h"
#include "service/EventService.h"
#include "CropManager.h"
#include "FishingSystem.h"
#include "GameTime.h"
#include "cocos2d.h"

using namespace cocos2d;

void MapService::init(Scene* scn, const std::string& initialMap) {
    scene = scn;
    gameMap = GameMap::create(initialMap);
    if (gameMap && scene) {
        scene->addChild(gameMap);
    }
}

void MapService::switchMap(const std::string& mapName, const Vec2& targetTilePos, PlayerService* ps) {
    if (!gameMap || !ps || !scene) return;
    
    // 如果当前是农场地图，保存作物信息
    if (gameMap->getMapName() == "Farm") {
        CropManager::getInstance()->saveCrops();
        CropManager::getInstance()->clearCrops();
    }
    
    // 保存当前玩家的引用
    Player* currentPlayer = ps->getPlayer();
    if (currentPlayer) {
        currentPlayer->retain();
        currentPlayer->removeFromParent();
    }
    
    // 移除旧地图的显示
    if (gameMap) {
        gameMap->getTileMap()->removeFromParent();
    }
    
    // 获取当前时间
    auto gameTime = GameTime::getInstance();
    int currentMonth = gameTime->getMonth();
    int currentDay = gameTime->getDay();
    
    // 检查是否是切换到Town地图
    std::string actualMapName = mapName;
    if (mapName == "Town") {
        // 如果是3月3日，传送到Town_Christmas
        if (currentMonth == 3 && currentDay == 3) {
            actualMapName = "Town_Christmas";
            CCLOG("Switching to Town_Christmas map");
            try {
                gameMap->loadMap("Town_Christmas");
            } catch (const std::exception& e) {
                CCLOG("wrong Path");
                gameMap->loadMap("Town");
                actualMapName = "Town";
            }
        } else {
            CCLOG("Switching to Town map");
            gameMap->loadMap("Town");
        }
    } else {
        gameMap->loadMap(mapName);
    }
    
    // 播放BGM
    if (audioService) {
        audioService->playBGMForMap(actualMapName, currentMonth, currentDay);
    }
    
    // 重用现有玩家，而不是创建新的
    if (currentPlayer) {
        const Vec2 worldPos = gameMap->convertToWorldCoord(targetTilePos);
        currentPlayer->setPosition(worldPos);
        currentPlayer->setGameMap(gameMap);
        scene->addChild(currentPlayer, 1);
        currentPlayer->release();
    }
    
    // 更新CropManager的地图引用
    CropManager::getInstance()->setGameMap(gameMap);
    
    // 如果切换到农场地图，加载作物
    if (actualMapName == "Farm") {
        CropManager::getInstance()->loadCrops();
        CCLOG("Switching to Farm map, loading crops...");
    }
    
    // 重新初始化钓鱼系统
    FishingSystem::getInstance()->initFishingAreas(gameMap);
    
    // 通知其他服务地图已切换
    if (uiService) {
        uiService->onMapChanged(actualMapName);
    }
    if (eventService) {
        eventService->onMapChanged(actualMapName);
    }
}

void MapService::postUpdate(float dt, PlayerService* ps) {
    if (!gameMap || !ps) return;
    Player* player = ps->getPlayer();
    if (!player) return;
    // 相机与前景遮挡
    gameMap->updateFrontTileVisibility(player->getPosition());
}

bool MapService::checkTransition(Vec2& targetMap, Vec2& targetTilePos) {
    if (!gameMap) return false;
    
    // 这里需要PlayerService来获取玩家位置
    // 由于接口限制，这个方法需要在GameFacade中调用
    return false;  // 占位实现
}