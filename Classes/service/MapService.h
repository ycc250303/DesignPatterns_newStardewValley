#pragma once
#include "cocos2d.h"
#include <string>
class GameMap;
class PlayerService;
class AudioService;
class UIService;
class EventService;

class MapService {
public:
    void init(cocos2d::Scene* scene, const std::string& initialMap);
    void switchMap(const std::string& mapName, const cocos2d::Vec2& targetTilePos, PlayerService* ps);
    void postUpdate(float dt, PlayerService* ps);
    GameMap* getMap() const { return gameMap; }
    
    // 设置依赖服务
    void setAudioService(AudioService* as) { audioService = as; }
    void setUIService(UIService* ui) { uiService = ui; }
    void setEventService(EventService* es) { eventService = es; }
    
    // 检查传送点
    bool checkTransition(cocos2d::Vec2& targetMap, cocos2d::Vec2& targetTilePos);
    
private:
    GameMap* gameMap = nullptr;
    cocos2d::Scene* scene = nullptr;
    AudioService* audioService = nullptr;
    UIService* uiService = nullptr;
    EventService* eventService = nullptr;
};