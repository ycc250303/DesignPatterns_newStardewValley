#pragma once
#include "cocos2d.h"
#include "Player.h"
#include "GameMap.h"
#include "InventoryUI.h"
#include "FishingSystem.h"
#include "StatusUI.h"
#include "BaseEvent.h"
#include "ItemSystem.h"
#include "EntityContainer.h"
#include <memory>

// 前向声明
class Player;
class GameMap;
class Chest;
class Lewis;
/*
 * 游戏场景类
 * 功能：管理游戏主场景的所有内容
 * 1.地图加载和显示
 * 2.玩家管理
 * 3.地图切换
 * 4.事件处理
 * 5.背包系统
 */
class GameScene : public cocos2d::Scene
{
public:
	// 核心函数声明
	static cocos2d::Scene* createScene();							// 创建并返回游戏场景实例
	virtual bool init();											// 初始化游戏场景
	CREATE_FUNC(GameScene);											// cocos2d场景创建宏
	virtual void update(float dt) override;							// 场景更新函数，每帧调用
	InventoryUI* getInventoryUI() const;							// 获取背包UI实例（通过UIService）
	Player* getPlayer() const { return player; }					// 获取玩家实例

	// 地图切换相关
	void switchToMap(const std::string& mapName, const cocos2d::Vec2& targetTilePos);	// 切换到指定地图和位置

	// 相机控制
	void updateCamera();											// 更新相机位置跟随玩家
	void updateQuestUIPosition();									// 更新任务UI的位置

	// 时间更新相关
	void onDayChanged();											// 处理日期变化时的更新
	void showDialogue(const std::string& dialogue, const std::string& characterImagePath, const std::string& characterName, int heartPoints);
	void toggleMute(Ref* sender);									// 切换静音状态

private:
	// 场景对象
	Player* player = nullptr;								// 玩家实例
	GameMap* _gameMap = nullptr;							// 当前游戏地图
	DialogueBox* dialogueBox = nullptr;						// 对话框实例
	cocos2d::TMXTiledMap* tileMap = nullptr;				// Tiled地图实例
	cocos2d::TMXObjectGroup* collisionsGroup = nullptr;		// 碰撞检测组
	
	// 实体容器（组合模式）
	std::unique_ptr<EntityContainer> entityContainer;
	
	// 宝箱管理（暂时保留，可后续纳入组合模式）
	std::vector<Chest*> _chests;													// 存储所有宝箱的实例
	void initChests();																// 初始化宝箱
	void clearChests();																// 清理所有宝箱

	// 矿洞进入记录
	int lastMineEnterDay = 0;										// 上次进入矿洞的日期
	int lastMineEnterMonth = 0;										// 上次进入矿洞的月份
	int lastMineEnterYear = 0;										// 上次进入矿洞的年份
	cocos2d::MenuItemImage* muteButton;								// 静音按钮
	bool isMuted;													// 当前静音状态
	std::vector<BaseEvent*> _events;								// 存储所有事件实例

	// 输入控制
	std::set<char> _pressedKeys;                                    // 当前按下的按键集合
	cocos2d::EventListenerKeyboard* _keyboardListener = nullptr;    // 键盘事件监听器
	void initMouseListener();                                       // 初始化鼠标事件监听器

	// 任务系统UI
	Label* _questTipLabel = nullptr;								// 任务提示文本
	Sprite* _questMark = nullptr;									// 任务标记图标
	void updateQuestUI();											// 更新任务UI显示
	void showQuestMark(Node* target);								// 显示任务标记
	void hideQuestMark(Node* target);								// 隐藏任务标记
	void handleQuestDialogue(Lewis* lewis);							// 处理与Lewis的任务对话
	void handleWoodQuest(Lewis* lewis, QuestState questState);		// 处理木材收集任务
	void handleBridgeQuest(Lewis* lewis, QuestState questState);	// 处理桥梁修复任务
	void checkQuestProgress();										// 检查任务进度
};
