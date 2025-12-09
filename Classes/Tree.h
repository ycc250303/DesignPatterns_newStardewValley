#pragma once
#include "cocos2d.h"
#include "GameEntity.h"
#include <functional>

USING_NS_CC;

class GameScene;

class Tree : public cocos2d::Sprite, public GameEntity {
public:
    // 定义回调函数类型
    typedef std::function<void()> TreeCallback;

    // 创建树的静态方法
    static Tree* create(const std::string& spriteName, int health = 100);

    Tree();
    virtual ~Tree();

    // 初始化树
    virtual bool init(const std::string& spriteName, int health);

    // 砍伐树的方法，返回是否砍倒
    bool chop(int damage);

    // 获取当前生命值
    int getHealth() const { return _health; }

    // 是否可以砍伐
    bool canBeChopped() const { return !_chopped && _canChop; }

    // 树是否被砍倒
    bool isChopped() const { return _chopped; }

    // 创建掉落的木头
    void createWoodDrop();

    // 设置树木掉落物品的回调
    void setOnTreeChoppedCallback(const TreeCallback& callback) {
        _onTreeChopped = callback;
    }
    
    // 实现 GameEntity 接口
    void initialize(const cocos2d::Vec2& tilePos, GameMap* map) override;
    void update(float dt) override;
    void cleanup() override;
    std::string getEntityType() const override { return "resource"; }
    std::string getEntityId() const override;
    bool shouldSpawnOnMap(const std::string& mapName) const override { return mapName == "Farm"; }
    void setVisible(bool visible) override { Node::setVisible(visible); }
    
    // 设置场景引用（用于回调）
    void setGameScene(GameScene* scene) { _gameScene = scene; }

private:
    static int nextId;
    int entityId;
    int _health;  // 树的生命值
    bool _chopped; // 树是否被砍倒
    bool _canChop; // 是否可以砍伐
    TreeCallback _onTreeChopped; // 树被砍倒时的回调函数
    GameScene* _gameScene = nullptr; // 场景引用
};
