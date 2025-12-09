#pragma once
#include "NPC.h"
#include "GameEntity.h"

// 马龙 NPC 类
class Marlon : public NPC, public GameEntity
{
public:
    static Marlon* create();
    virtual bool init() override;

    void staticAnimation() override;
    
    // 实现 GameEntity 接口
    void initialize(const cocos2d::Vec2& tilePos, GameMap* map) override;
    void update(float dt) override;
    void cleanup() override;
    std::string getEntityType() const override { return "npc"; }
    std::string getEntityId() const override { return "marlon"; }
    bool shouldSpawnOnMap(const std::string& mapName) const override { return mapName == "Mine"; }
    void setVisible(bool visible) override { Node::setVisible(visible); }
    
private:
    void initializeDefaultBehavior(); // 初始化马龙 NPC 的默认行为
    void completeDialogue()
    {
        this->addDialogue("Can't resist the urge for adventure? Great, today's challenge is here.");
        this->addDialogue("The light of courage shines in your eyes. How about a challenge?");
        this->addDialogue("Though I can only see with one eye, I will not miss any lurking beast.");
        this->addDialogue("Don't get too close to the cage...");
    }
};
