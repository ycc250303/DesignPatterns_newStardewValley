#pragma once
#include "NPC.h"
#include "GameEntity.h"

class Sheep :public NPC, public GameEntity
{
public:
	static Sheep* create();
	virtual bool init();
	void moveToDirection(cocos2d::Vec2& destination, float dt);//移动到目的地
	void staticAnimation() override;//静止时的呼吸动画
	void moveAlongPath(float dt);//移动沿路径
	ActionState currentActionState = ActionState::MOVING; // 当前动作状态
	std::vector<cocos2d::Vec2> path; // 移动路径
	void setPath(const std::vector<cocos2d::Vec2>& newPath) {
		path = newPath;
		currentPathIndex = 0; // 重置路径索引
	}
	
	// 实现 GameEntity 接口
	void initialize(const cocos2d::Vec2& tilePos, GameMap* map) override;
	void initialize(const cocos2d::Vec2& tilePos, GameMap* map, const std::vector<cocos2d::Vec2>& path) override;
	void update(float dt) override;
	void cleanup() override;
	std::string getEntityType() const override { return "animal"; }
	std::string getEntityId() const override;
	bool shouldSpawnOnMap(const std::string& mapName) const override { return mapName == "Farm"; }
	void setVisible(bool visible) override { Node::setVisible(visible); }
	
private:
	static int nextId;
	int entityId;
	int currentPathIndex = 0; // 当前路径索引
	int currentDirection = 0;                   // 当前朝向(0:下, 1:上, 2:左, 3:右)
	int currentFrame = 0;                       // 当前帧索引
	float animationTimer = 0;                   // 动画计时器
	const float FRAME_INTERVAL = 0.3f;          // 帧间隔时间
};
