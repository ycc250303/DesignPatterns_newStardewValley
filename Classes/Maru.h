#pragma once
#include "NPC.h"
#include "GameEntity.h"

// 玛鲁 NPC 类
class Maru : public NPC, public GameEntity
{
public:
    static Maru* create();
    virtual bool init();
    void staticAnimation() override;
    
    // 实现 GameEntity 接口
    void initialize(const cocos2d::Vec2& tilePos, GameMap* map) override;
    void update(float dt) override;
    void cleanup() override;
    std::string getEntityType() const override { return "npc"; }
    std::string getEntityId() const override { return "maru"; }
    bool shouldSpawnOnMap(const std::string& mapName) const override { return mapName == "Hospital"; }
    void setVisible(bool visible) override { Node::setVisible(visible); }
    
private:
    void initializeDefaultBehavior(); // 初始化玛鲁 NPC 的默认行为
    void completeDialogue()
    {
        this->addDialogue("Welcome to Stardew Valley Hospital. Come to me at anytime.");
        this->addDialogue("If you're sick, remember to drink plenty of water and eat healthily.");
        this->addDialogue("Oh, you're injured? Let me check it out.");
        this->addDialogue("While it's quiet, there are occasionally situations that need attention.");
    }
};
