#include "UIService.h"
#include "InventoryUI.h"
#include "StatusUI.h"
#include "Player.h"
#include "service/PlayerService.h"
#include "service/MapService.h"
#include "GameMap.h"
#include "ItemSystem.h"
#include "DialogueBox.h"
#include "QuestSystem.h"
#include "GameTime.h"
#include "cocos2d.h"

using namespace cocos2d;

void UIService::init(Scene* scn, PlayerService* ps, MapService* ms) {
    scene = scn; playerService = ps; mapService = ms;
    dialogueBox = nullptr;
    questTipLabel = nullptr;
    questMark = nullptr;

    inventoryUI = InventoryUI::create();
    if (inventoryUI) {
        scene->addChild(inventoryUI, 10);
        inventoryUI->setVisible(false);
        ItemSystem::getInstance()->setUpdateCallback([this]() {
            if (inventoryUI) inventoryUI->updateDisplay();
        });
    }

    statusUI = StatusUI::create();
    if (statusUI) {
        const Size vs = Director::getInstance()->getVisibleSize();
        const Vec2 origin = Director::getInstance()->getVisibleOrigin();
        statusUI->setPosition(origin.x + vs.width, origin.y + vs.height);
        scene->addChild(statusUI, 10);
    }

    initToolIcon();
    initSeedIcon();
}

void UIService::initToolIcon() {
    toolIcon = Sprite::create("tools.png");
    if (!toolIcon) return;
    toolIcon->setScale(3.0f);
    scene->addChild(toolIcon, 10);
    toolIcon->setPosition(Player::TOOL_ICON_POSITION);
    updateToolIcon();
}

void UIService::updateToolIcon() {
    if (!toolIcon || !playerService) return;
    Player* player = playerService->getPlayer();
    if (!player) return;
    int toolIndex = static_cast<int>(player->getCurrentTool());
    switch (toolIndex) {
    case 0: toolIcon->setVisible(false); break;
    case 1: toolIcon->setTexture("tools.png"); toolIcon->setVisible(true); toolIcon->setTextureRect(Rect(32,0,16,16)); break;
    case 2: toolIcon->setTexture("tools.png"); toolIcon->setVisible(true); toolIcon->setTextureRect(Rect(16,0,16,16)); break;
    case 3: toolIcon->setTexture("TileSheets/tools.png"); toolIcon->setVisible(true); toolIcon->setTextureRect(Rect(80,96,16,16)); break;
    case 4: toolIcon->setTexture("tools.png"); toolIcon->setVisible(true); toolIcon->setTextureRect(Rect(0,0,16,16)); break;
    case 5: toolIcon->setTexture("TileSheets/Tools.png"); toolIcon->setVisible(true); toolIcon->setTextureRect(Rect(128,0,16,16)); break;
    case 6: toolIcon->setTexture("TileSheets/Objects_2.png"); toolIcon->setVisible(true); toolIcon->setTextureRect(Rect(96,32,16,16)); break;
    case 7: toolIcon->setTexture("LooseSprites/emojis.png"); toolIcon->setVisible(true); toolIcon->setTextureRect(Rect(18,36,9,9)); break;
    default: toolIcon->setVisible(false); break;
    }
}

void UIService::initSeedIcon() {
    seedIcon = Sprite::create("Plants.png");
    if (!seedIcon) return;
    seedIcon->setScale(3.0f);
    scene->addChild(seedIcon, 10);
    seedIcon->setPosition(Player::SEED_ICON_POSITION);
    seedIcon->setVisible(mapService && mapService->getMap() && mapService->getMap()->getMapName() == "Farm");
    updateSeedIcon();
}

void UIService::updateSeedIcon() {
    if (!seedIcon || !playerService) return;
    Player* player = playerService->getPlayer();
    if (!player) return;
    if (mapService && mapService->getMap()) {
        seedIcon->setVisible(mapService->getMap()->getMapName() == "Farm");
    }
    switch (player->getCurrentSeed()) {
    case Player::SeedType::CORN:   seedIcon->setTextureRect(Rect(0,0,16,16)); break;
    case Player::SeedType::TOMATO: seedIcon->setTextureRect(Rect(0,16,16,16)); break;
    default: seedIcon->setVisible(false); break;
    }
}

void UIService::update(float dt) {
    updateToolIcon();
    updateSeedIcon();
}

void UIService::onMapChanged(const std::string& mapName) {
    if (seedIcon) seedIcon->setVisible(mapName == "Farm");
}

cocos2d::Vec2 UIService::calculateCameraOffset() {
    if (!playerService || !mapService) return Vec2::ZERO;
    
    Player* player = playerService->getPlayer();
    GameMap* gameMap = mapService->getMap();
    if (!player || !gameMap) return Vec2::ZERO;
    
    if (gameMap->getMapName() == "First") return Vec2::ZERO;
    
    const Size visibleSize = Director::getInstance()->getVisibleSize();
    const Size mapSize = gameMap->getTileMap()->getMapSize();
    const Size tileSize = gameMap->getTileMap()->getTileSize();
    const float scale = gameMap->getTileMap()->getScale();
    
    const float mapWidth = mapSize.width * tileSize.width * scale;
    const float mapHeight = mapSize.height * tileSize.height * scale;
    const Vec2 playerPos = player->getPosition();
    
    float x, y;
    if (mapWidth < visibleSize.width) {
        x = visibleSize.width / 2;
    } else {
        x = std::max(playerPos.x, visibleSize.width / 2);
        x = std::min(x, mapWidth - visibleSize.width / 2);
    }
    if (mapHeight < visibleSize.height) {
        y = visibleSize.height / 2;
    } else {
        y = std::max(playerPos.y, visibleSize.height / 2);
        y = std::min(y, mapHeight - visibleSize.height / 2);
    }
    
    const Vec2 pointA = Vec2(visibleSize.width / 2, visibleSize.height / 2);
    const Vec2 pointB = Vec2(x, y);
    Vec2 offset = pointA - pointB;
    
    if (mapWidth < visibleSize.width) {
        offset.x = (visibleSize.width - mapWidth) / 2;
    }
    if (mapHeight < visibleSize.height) {
        offset.y = (visibleSize.height - mapHeight) / 2;
    }
    
    return offset;
}

void UIService::updateCameraAndUI() {
    if (!scene || !playerService || !mapService) return;
    
    Vec2 offset = calculateCameraOffset();
    scene->setPosition(offset);
    
    // 更新背包UI位置
    if (inventoryUI) {
        inventoryUI->setPosition(-offset);
    }
    
    // 更新工具图标位置
    if (toolIcon) {
        toolIcon->setPosition(-offset + Vec2(50, 50));
    }
    
    // 更新种子图标位置
    if (seedIcon) {
        seedIcon->setPosition(-offset + Vec2(100, 50));
    }
    
    // 更新对话框位置
    if (dialogueBox) {
        const Size visibleSize = Director::getInstance()->getVisibleSize();
        dialogueBox->setPosition(-offset + Vec2(visibleSize.width / 2, 90));
    }
    
    // 更新状态UI位置
    if (statusUI) {
        const Size visibleSize = Director::getInstance()->getVisibleSize();
        statusUI->setPosition(-offset + Vec2(visibleSize.width, visibleSize.height));
    }
    
    // 更新任务UI位置
    updateQuestUIPosition();
}

void UIService::updateQuestUI() {
    if (!questTipLabel) return;
    
    auto questSystem = QuestSystem::getInstance();
    auto gameTime = GameTime::getInstance();
    int currentDay = gameTime->getDay();
    
    if (currentDay == 1 &&
        questSystem->getQuestState(QuestType::COLLECT_WOOD) == QuestState::IN_PROGRESS) {
        auto& questData = questSystem->getQuestData(QuestType::COLLECT_WOOD);
        int woodCount = ItemSystem::getInstance()->getItemCount("wood");
        questTipLabel->setString(questData.title + ": " +
            std::to_string(woodCount) + "/" +
            std::to_string(questData.targetAmount));
    } else if (currentDay == 2 &&
        questSystem->getQuestState(QuestType::REPAIR_BRIDGE) == QuestState::IN_PROGRESS) {
        auto& questData = questSystem->getQuestData(QuestType::REPAIR_BRIDGE);
        questTipLabel->setString(questData.title);
    }
}

void UIService::updateQuestUIPosition() {
    if (!questTipLabel) return;
    
    const Size visibleSize = Director::getInstance()->getVisibleSize();
    const Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 offset = calculateCameraOffset();
    
    Vec2 position = Vec2(origin.x + visibleSize.width - 20,
        origin.y + visibleSize.height - 160);
    position = position - offset;
    
    questTipLabel->setPosition(position);
}

void UIService::showQuestMark(cocos2d::Node* target) {
    if (!target) return;
    
    if (!questMark) {
        questMark = Sprite::create("LooseSprites/emojis.png");
        if (questMark) {
            questMark->setTextureRect(Rect(54, 81, 9, 9));
            questMark->setScale(1.5f);
            questMark->setPosition(Vec2(10, target->getContentSize().height));
            target->addChild(questMark);
            
            auto moveUp = MoveBy::create(0.5f, Vec2(0, 10));
            auto moveDown = moveUp->reverse();
            auto sequence = Sequence::create(moveUp, moveDown, nullptr);
            auto repeat = RepeatForever::create(sequence);
            questMark->runAction(repeat);
        }
    }
    if (questMark) {
        questMark->setVisible(true);
    }
}

void UIService::hideQuestMark() {
    if (questMark) {
        questMark->setVisible(false);
    }
}

void UIService::setQuestTipLabelVisible(bool visible) {
    if (questTipLabel) {
        questTipLabel->setVisible(visible);
        if (!visible) {
            questTipLabel->removeFromParent();
            questTipLabel = nullptr;
        }
    }
}

void UIService::createQuestTipLabelIfNeeded() {
    if (questTipLabel) return;  // 已存在则不需要创建
    
    auto questSystem = QuestSystem::getInstance();
    auto gameTime = GameTime::getInstance();
    int currentDay = gameTime->getDay();
    
    bool shouldCreate = false;
    if ((currentDay == 1 && questSystem->getQuestState(QuestType::COLLECT_WOOD) == QuestState::IN_PROGRESS) ||
        (currentDay == 2 && questSystem->getQuestState(QuestType::REPAIR_BRIDGE) == QuestState::IN_PROGRESS)) {
        shouldCreate = true;
    }
    
    if (shouldCreate && scene) {
        questTipLabel = Label::createWithTTF("", "fonts/arial.ttf", 24);
        if (questTipLabel) {
            scene->addChild(questTipLabel, 10);
            questTipLabel->setAnchorPoint(Vec2(1, 1));
            updateQuestUIPosition();
            updateQuestUI();
        }
    }
}

void UIService::setDialogueBox(DialogueBox* box) {
    dialogueBox = box;
}