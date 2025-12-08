#pragma once
#include <vector>
class BaseEvent;
class MapService;
class PlayerService;
class Lewis;
class UIService;
class QuestSystem;
enum class QuestState;

class EventService {
public:
    void init(cocos2d::Scene* scene, MapService* ms, PlayerService* ps);
    void update(float dt);
    void onDayChanged();
    void onMapChanged(const std::string& mapName);
    
    // 任务相关
    void handleQuestDialogue(Lewis* lewis);
    void checkQuestProgress();
    void setUIService(UIService* ui) { uiService = ui; }
    
private:
    std::vector<BaseEvent*> events;
    cocos2d::Scene* scene = nullptr;
    MapService* mapService = nullptr;
    PlayerService* playerService = nullptr;
    UIService* uiService = nullptr;
    
    void handleWoodQuest(Lewis* lewis, QuestState questState);
    void handleBridgeQuest(Lewis* lewis, QuestState questState);
};