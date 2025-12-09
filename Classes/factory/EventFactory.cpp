#include "factory/EventFactory.h"
#include "Sleep.h"
#include "BridgeEvent.h"
#include "Cooking.h"
#include <unordered_map>
#include <string>

class SleepEventCreator : public EventCreator {
public:
    BaseEvent* create(GameMap* gameMap, Player* player) const override {
        return SleepEvent::create(gameMap, player);
    }
};

class BridgeEventCreator : public EventCreator {
public:
    BaseEvent* create(GameMap* gameMap, Player* player) const override {
        return BridgeEvent::create(gameMap, player);
    }
};

class CookingEventCreator : public EventCreator {
public:
    BaseEvent* create(GameMap* gameMap, Player* player) const override {
        return Cooking::create(gameMap, player);
    }
};

/****************************************************************
 *
 * 使用工厂方法模式重构 - 重构后代码
 *
 ****************************************************************/
BaseEvent* EventFactory::create(const std::string& type, GameMap* gameMap, Player* player) {
    static SleepEventCreator sleepCreator;
    static BridgeEventCreator bridgeCreator;
    static CookingEventCreator cookingCreator;
    static const std::unordered_map<std::string, EventCreator*> creators = {
        { "sleep", &sleepCreator },
        { "bridge", &bridgeCreator },
        { "cooking", &cookingCreator }
    };

    auto it = creators.find(type);
    if (it != creators.end() && it->second) {
        return it->second->create(gameMap, player);
    }
    return nullptr;
}
