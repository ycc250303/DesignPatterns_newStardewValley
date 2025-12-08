#pragma once
#include "cocos2d.h"
#include <string>

class GameMap;
class Player;
struct TransitionInfo;

class GameFacade {
public:
    static GameFacade& instance();

    void initialize(cocos2d::Scene* scene);
    void update(float dt);
    void switchMap(const std::string& mapName, const cocos2d::Vec2& targetTilePos);
    
    // 日期变化处理
    void onDayChanged();
    
    // 检查传送点
    bool checkTransition(std::string& targetMap, cocos2d::Vec2& targetTilePos);
    
    // 获取服务实例
    Player* getPlayer() const;
    GameMap* getMap() const;
    class UIService* getUIService() const { return uiService; }
    class AudioService* getAudioService() const { return audioService; }
    class EventService* getEventService() const { return eventService; }

private:
    GameFacade() = default;
    ~GameFacade() = default;
    GameFacade(const GameFacade&) = delete;
    GameFacade& operator=(const GameFacade&) = delete;

    class MapService* mapService = nullptr;
    class PlayerService* playerService = nullptr;
    class EntityService* entityService = nullptr;
    class UIService* uiService = nullptr;
    class EventService* eventService = nullptr;
    class AudioService* audioService = nullptr;
    class WeatherService* weatherService = nullptr;
};