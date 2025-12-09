#include "GameFacade.h"
#include "service/MapService.h"
#include "service/PlayerService.h"
#include "service/UIService.h"
#include "service/EventService.h"
#include "service/AudioService.h"
#include "service/WeatherService.h"
#include "EventBus.h"
#include "GameMap.h"
#include "Player.h"
#include "Lewis.h"
#include "GameTime.h"
#include "LightManager.h"
#include "ItemSystem.h"
#include "FishingSystem.h"
#include "CropManager.h"
#include "InventoryUI.h"
#include "cocos2d.h"
#include <utility>

using namespace cocos2d;

namespace {
// 简单的事件订阅RAII封装：构造时保存订阅ID，析构时自动取消订阅
class EventBusSubscription {
public:
    EventBusSubscription(EventType type, int id)
        : type(type), id(id) {}
    ~EventBusSubscription() {
        if (id != -1) {
            EventBus::getInstance().unsubscribe(type, id);
        }
    }
    // 不允许拷贝，允许移动
    EventBusSubscription(const EventBusSubscription&) = delete;
    EventBusSubscription& operator=(const EventBusSubscription&) = delete;
    EventBusSubscription(EventBusSubscription&& other) noexcept
        : type(other.type), id(other.id) {
        other.id = -1;
    }
    EventBusSubscription& operator=(EventBusSubscription&& other) noexcept {
        if (this != &other) {
            // 先取消自身，再接管
            if (id != -1) {
                EventBus::getInstance().unsubscribe(type, id);
            }
            type = other.type;
            id = other.id;
            other.id = -1;
        }
        return *this;
    }
private:
    EventType type;
    int id;
};
} // namespace

GameFacade& GameFacade::instance() {
    static GameFacade inst;
    return inst;
}

void GameFacade::initialize(cocos2d::Scene* scene) {
/****************************************************************
 *
 * 使用RAII模式重构-重构后的代码
 *
 ****************************************************************/
    mapService = std::make_unique<MapService>();
    playerService = std::make_unique<PlayerService>();
    uiService = std::make_unique<UIService>();
    eventService = std::make_unique<EventService>();
    audioService = std::make_unique<AudioService>();
    weatherService = std::make_unique<WeatherService>();

    mapService->init(scene, "First");
    playerService->init(scene, mapService.get());
    uiService->init(scene, playerService.get(), mapService.get());
    eventService->init(scene, mapService.get(), playerService.get());
    
    // 设置服务之间的依赖关系
    mapService->setAudioService(audioService.get());
    mapService->setUIService(uiService.get());
    mapService->setEventService(eventService.get());
    eventService->setUIService(uiService.get());
    eventService->setWeatherService(weatherService.get());
    
    weatherService->init();
    audioService->init();

    auto& bus = EventBus::getInstance();
/****************************************************************
 *
 * 使用RAII模式重构-重构后的代码
 *
 ****************************************************************/
    dayChangedSubscription = std::make_unique<EventBusSubscription>(
        EventType::DayChanged,
        bus.subscribe(EventType::DayChanged, [this](const Event& e) {
            const auto* payload = static_cast<const DayChangedEvent*>(e.data);
            if (payload && eventService) {
                eventService->onDayChanged(*payload);
            }
            if (payload && weatherService) {
                weatherService->onDayChanged(*payload);
            }
        })
    );
    mapSwitchedSubscription = std::make_unique<EventBusSubscription>(
        EventType::MapSwitched,
        bus.subscribe(EventType::MapSwitched, [this](const Event& e) {
            const auto* payload = static_cast<const MapSwitchedEvent*>(e.data);
        })
    );
    
    // 初始化物品系统（从GameScene迁移）
    ItemSystem* itemSystem = ItemSystem::getInstance();
    itemSystem->addItem("corn seed", 5);
    itemSystem->addItem("tomato seed", 5);
    itemSystem->addItem("fertilizer", 5);
}

void GameFacade::update(float dt) {
    // 更新游戏时间
    GameTime* gameTime = GameTime::getInstance();
    static int lastDay = gameTime->getDay();
    gameTime->update();
    
    // 检查日期变化
    if (gameTime->getDay() != lastDay) {
        onDayChanged();
        lastDay = gameTime->getDay();
    }
    
    // 更新各个服务
    weatherService->update(dt);
    audioService->update(dt);
    playerService->update(dt);
    eventService->update(dt);
    uiService->update(dt);
    
    // 检查任务进度
    eventService->checkQuestProgress();
    
    // 更新地图相关（相机、前景遮挡等）
    mapService->postUpdate(dt, playerService);
    
    // 更新UI相机位置
    uiService->updateCameraAndUI();
    
    // 更新光照效果
    LightManager::getInstance()->update();
}

void GameFacade::switchMap(const std::string& mapName, const cocos2d::Vec2& targetTilePos) {
    mapService->switchMap(mapName, targetTilePos, playerService);
}

void GameFacade::onDayChanged() {
    auto gameTime = GameTime::getInstance();
    DayChangedEvent payload{gameTime->getYear(), gameTime->getMonth(), gameTime->getDay()};
    EventBus::getInstance().publish(Event{EventType::DayChanged, &payload});
}

bool GameFacade::checkTransition(std::string& targetMap, cocos2d::Vec2& targetTilePos) {
    if (!mapService || !playerService) return false;
    
    GameMap* gameMap = mapService->getMap();
    Player* player = playerService->getPlayer();
    if (!gameMap || !player) return false;
    
    Vec2 playerTilePos = gameMap->convertToTileCoord(player->getPosition());
    TransitionInfo transition;
    if (gameMap->checkForTransition(playerTilePos, transition)) {
        targetMap = transition.targetMap;
        targetTilePos = transition.targetTilePos;
        return true;
    }
    return false;
}

Player* GameFacade::getPlayer() const { 
    return playerService ? playerService->getPlayer() : nullptr; 
}

GameMap* GameFacade::getMap() const { 
    return mapService ? mapService->getMap() : nullptr; 
}

// ========== 音频服务封装实现 ==========
void GameFacade::toggleMute() {
    if (audioService) {
        audioService->toggleMute();
    }
}

bool GameFacade::getIsMuted() const {
    return audioService ? audioService->getIsMuted() : false;
}

// ========== UI服务封装实现 ==========
void GameFacade::showQuestMark(cocos2d::Node* target) {
    if (uiService && target) {
        uiService->showQuestMark(target);
    }
}

void GameFacade::hideQuestMark() {
    if (uiService) {
        uiService->hideQuestMark();
    }
}

void GameFacade::setQuestTipLabelVisible(bool visible) {
    if (uiService) {
        uiService->setQuestTipLabelVisible(visible);
    }
}

void GameFacade::createQuestTipLabelIfNeeded() {
    if (uiService) {
        uiService->createQuestTipLabelIfNeeded();
    }
}

void GameFacade::updateQuestUI() {
    if (uiService) {
        uiService->updateQuestUI();
    }
}

void GameFacade::updateQuestUIPosition() {
    if (uiService) {
        uiService->updateQuestUIPosition();
    }
}

InventoryUI* GameFacade::getInventoryUI() const {
    return uiService ? uiService->getInventoryUI() : nullptr;
}

// ========== 事件服务封装实现 ==========
void GameFacade::handleQuestDialogue(Lewis* lewis) {
    if (eventService && lewis) {
        eventService->handleQuestDialogue(lewis);
    }
}

void GameFacade::checkQuestProgress() {
    if (eventService) {
        eventService->checkQuestProgress();
    }
}

// ========== 天气服务封装实现 ==========
void GameFacade::randomRefreshWeather() {
    if (weatherService) {
        weatherService->randomRefreshWeather();
    }
}

void GameFacade::setWeather(const std::string& type) {
    if (weatherService) {
        weatherService->setWeather(type);
    }
}
