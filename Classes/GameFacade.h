#pragma once
#include "cocos2d.h"
#include "EventBus.h"
#include <string>
#include <memory>

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
    
    // ========== 地图服务封装 ==========
    // 获取核心游戏对象
    Player* getPlayer() const;
    GameMap* getMap() const;
    
    // ========== 音频服务封装 ==========
    void toggleMute();
    bool getIsMuted() const;
    
    // ========== UI服务封装 ==========
    void showQuestMark(cocos2d::Node* target);
    void hideQuestMark();
    void setQuestTipLabelVisible(bool visible);
    void createQuestTipLabelIfNeeded();
    void updateQuestUI();
    void updateQuestUIPosition();
    class InventoryUI* getInventoryUI() const;
    
    // ========== 事件服务封装 ==========
    void handleQuestDialogue(class Lewis* lewis);
    void checkQuestProgress();
    
    // ========== 天气服务封装 ==========
    void randomRefreshWeather();
    void setWeather(const std::string& type); // "rain"/"drought"/"normal"

private:
    GameFacade() = default;
    ~GameFacade() = default;
    GameFacade(const GameFacade&) = delete;
    GameFacade& operator=(const GameFacade&) = delete;

/****************************************************************
 *
 * 使用RAII模式重构-重构后的代码
 *
 ****************************************************************/
    std::unique_ptr<class MapService> mapService;
    std::unique_ptr<class PlayerService> playerService;
    std::unique_ptr<class UIService> uiService;
    std::unique_ptr<class EventService> eventService;
    std::unique_ptr<class AudioService> audioService;
    std::unique_ptr<class WeatherService> weatherService;
/****************************************************************
 *
 * 使用RAII模式重构-重构后的代码
 *
 ****************************************************************/
    std::unique_ptr<class EventBusSubscription> dayChangedSubscription;
    std::unique_ptr<class EventBusSubscription> mapSwitchedSubscription;
    
    // 内部服务访问器（仅供内部使用，不对外暴露）
    class UIService* getUIService() const { return uiService.get(); }
    class AudioService* getAudioService() const { return audioService.get(); }
    class EventService* getEventService() const { return eventService.get(); }
};
