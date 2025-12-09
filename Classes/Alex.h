#pragma once
#include "NPC.h"
#include "GameEntity.h"

// 艾利克斯 NPC 类
class Alex : public NPC, public GameEntity
{
public:
    static Alex* create();
    virtual bool init() override;
    void staticAnimation() override;
    void initializeAnimations();
    
    // 实现 GameEntity 接口
    void initialize(const cocos2d::Vec2& tilePos, GameMap* map) override;
    void update(float dt) override;
    void cleanup() override;
    std::string getEntityType() const override { return "npc"; }
    std::string getEntityId() const override { return "alex"; }
    bool shouldSpawnOnMap(const std::string& mapName) const override { return mapName == "Town"; }
    void setVisible(bool visible) override { Node::setVisible(visible); }
    
private:
    void initializeDefaultBehavior(); // 初始化艾利克斯 NPC 的默认行为
    void completeDialogue()
    {
        this->addDialogue("Today is a great day to train!");
        this->addDialogue("Hey, farmer!If you want to get stronger, you should do more sports.");
        this->addDialogue("I like working out with my friends, especially Maru.");
        this->addDialogue("Nice weather today. Perfect for getting a good workout in.");
    }
};
