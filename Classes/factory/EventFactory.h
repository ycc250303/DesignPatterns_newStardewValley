#pragma once
#include <string>
class BaseEvent;
class GameMap;
class Player;

class EventCreator {
public:
    virtual ~EventCreator() = default;
    virtual BaseEvent* create(GameMap* gameMap, Player* player) const = 0;
};

class EventFactory {
public:
    static BaseEvent* create(const std::string& type, GameMap* gameMap, Player* player);
};
