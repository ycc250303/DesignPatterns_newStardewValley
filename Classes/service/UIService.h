#pragma once
#include "cocos2d.h"
class PlayerService;
class MapService;
class InventoryUI;
class StatusUI;
class DialogueBox;

class UIService {
public:
    void init(cocos2d::Scene* scene, PlayerService* ps, MapService* ms);
    void update(float dt);
    void onMapChanged(const std::string& mapName);
    
    // 相机和UI位置更新
    void updateCameraAndUI();
    
    // 任务UI管理
    void updateQuestUI();
    void updateQuestUIPosition();
    void showQuestMark(cocos2d::Node* target);
    void hideQuestMark();
    void setQuestTipLabelVisible(bool visible);
    void createQuestTipLabelIfNeeded();  // 如果需要，创建任务提示标签
    
    // 对话框管理
    void setDialogueBox(DialogueBox* box);
    DialogueBox* getDialogueBox() const { return dialogueBox; }
    
    // 获取UI组件（供外部访问）
    InventoryUI* getInventoryUI() const { return inventoryUI; }
    StatusUI* getStatusUI() const { return statusUI; }
    
private:
    cocos2d::Scene* scene = nullptr;
    PlayerService* playerService = nullptr;
    MapService* mapService = nullptr;
    InventoryUI* inventoryUI = nullptr;
    StatusUI* statusUI = nullptr;
    cocos2d::Sprite* toolIcon = nullptr;
    cocos2d::Sprite* seedIcon = nullptr;
    DialogueBox* dialogueBox = nullptr;
    
    // 任务UI
    cocos2d::Label* questTipLabel = nullptr;
    cocos2d::Sprite* questMark = nullptr;

    void initToolIcon();
    void updateToolIcon();
    void initSeedIcon();
    void updateSeedIcon();
    cocos2d::Vec2 calculateCameraOffset();
};