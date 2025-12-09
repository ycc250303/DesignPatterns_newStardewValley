#pragma once
#include "cocos2d.h"
#include "GameEntity.h"
#include <functional>

class GameScene;

class Ore : public cocos2d::Sprite, public GameEntity
{
public:
    static Ore* create(const std::string& spriteName, int health);
    bool canBeDug() const { return _canDug; }
    void dig(int damage);
    void remove();

    // 设置挖掘回调
    void setOnOreDug(const std::function<void()>& callback) { _onOreDug = callback; }
    
    // 实现 GameEntity 接口
    void initialize(const cocos2d::Vec2& tilePos, GameMap* map) override;
    void update(float dt) override;
    void cleanup() override;
    std::string getEntityType() const override { return "resource"; }
    std::string getEntityId() const override;
    bool shouldSpawnOnMap(const std::string& mapName) const override { return mapName == "Mine"; }
    void setVisible(bool visible) override { Node::setVisible(visible); }
    
    // 设置场景引用（用于回调）
    void setGameScene(GameScene* scene) { _gameScene = scene; }

private:
    Ore();
    ~Ore();
    bool init(const std::string& spriteName, int health);
    static int nextId;
    int entityId;
    int _health;
    bool _dug;
    bool _canDug;
    std::function<void()> _onOreDug;
    GameScene* _gameScene = nullptr;
};