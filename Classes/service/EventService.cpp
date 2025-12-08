#include "EventService.h"
#include "BaseEvent.h"
#include "Sleep.h"
#include "BridgeEvent.h"
#include "Cooking.h"
#include "service/MapService.h"
#include "service/PlayerService.h"
#include "service/UIService.h"
#include "GameMap.h"
#include "Player.h"
#include "Lewis.h"
#include "QuestSystem.h"
#include "ItemSystem.h"
#include "DialogueBox.h"
#include "GameTime.h"
#include "CropManager.h"
#include "WeatherManager.h"
#include "cocos2d.h"

using namespace cocos2d;

void EventService::init(Scene* scn, MapService* ms, PlayerService* ps) {
    scene = scn;
    mapService = ms;
    playerService = ps;
    uiService = nullptr;
    
    if (!scene || !mapService || !playerService) return;
    
    GameMap* gameMap = mapService->getMap();
    Player* player = playerService->getPlayer();
    if (!gameMap || !player) return;
    
    // 创建事件
    events.push_back(SleepEvent::create(gameMap, player));
    events.push_back(BridgeEvent::create(gameMap, player));
    events.push_back(Cooking::create(gameMap, player));
    
    for (auto event : events) {
        if (event) {
            scene->addChild(event);
        }
    }
}

void EventService::update(float dt) {
    for (auto event : events) {
        if (event) {
            event->update(dt);
        }
    }
}

void EventService::onDayChanged() {
    if (!mapService) return;
    
    GameMap* gameMap = mapService->getMap();
    if (!gameMap) return;
    
    // 更新作物生长
    CropManager::getInstance()->updateCrops();
    
    // 刷新天气
    WeatherManager::getInstance()->randomRefreshWeather();
    
    // 刷新资源
    gameMap->refreshResources();
    
    // 重置任务状态
    auto questSystem = QuestSystem::getInstance();
    auto gameTime = GameTime::getInstance();
    int newDay = gameTime->getDay();
    
    // 第一天开始时，重置木头收集任务
    if (newDay == 1) {
        questSystem->resetQuest(QuestType::COLLECT_WOOD);
    }
    // 第二天开始时，重置修桥任务
    else if (newDay == 2) {
        questSystem->resetQuest(QuestType::REPAIR_BRIDGE);
    }
    
    // 移除任务提示UI（通过UIService）
    if (uiService) {
        uiService->setQuestTipLabelVisible(false);
    }
}

void EventService::onMapChanged(const std::string& mapName) {
    // 地图切换时可能需要重新初始化事件
    // 目前事件是全局的，不需要特殊处理
}

void EventService::handleQuestDialogue(Lewis* lewis) {
    if (!lewis || !scene) return;
    
    auto questSystem = QuestSystem::getInstance();
    if (!questSystem) return;
    
    auto gameTime = GameTime::getInstance();
    int currentDay = gameTime->getDay();
    
    // 第一天的木头收集任务
    if (currentDay == 1) {
        auto woodQuestState = questSystem->getQuestState(QuestType::COLLECT_WOOD);
        if (woodQuestState != QuestState::COMPLETED) {
            handleWoodQuest(lewis, woodQuestState);
            return;
        }
    }
    // 第二天的修桥任务
    else if (currentDay == 2) {
        auto bridgeQuestState = questSystem->getQuestState(QuestType::REPAIR_BRIDGE);
        if (bridgeQuestState != QuestState::COMPLETED) {
            handleBridgeQuest(lewis, bridgeQuestState);
            return;
        }
    }
    
    // 如果当天的任务已完成或是其他天，显示随机对话
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    auto dialogueBox = DialogueBox::create(lewis->getRandomDialogue(), "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
    scene->addChild(dialogueBox, 10);
    if (uiService) {
        uiService->setDialogueBox(dialogueBox);
    }
}

void EventService::handleWoodQuest(Lewis* lewis, QuestState questState) {
    if (!lewis || !scene) return;
    
    auto questSystem = QuestSystem::getInstance();
    DialogueBox* dialogueBox = nullptr;
    
    if (questState == QuestState::NOT_STARTED) {
        // 显示任务介绍对话
        auto& questData = questSystem->getQuestData(QuestType::COLLECT_WOOD);
        dialogueBox = DialogueBox::create(questData.description, "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
        scene->addChild(dialogueBox, 10);
        if (uiService) {
            uiService->setDialogueBox(dialogueBox);
        }
        
        // 开始任务
        questSystem->startQuest(QuestType::COLLECT_WOOD);
        if (uiService) {
            uiService->hideQuestMark();
            uiService->createQuestTipLabelIfNeeded();
            uiService->updateQuestUI();
        }
    }
    else if (questState == QuestState::IN_PROGRESS) {
        const int woodCount = ItemSystem::getInstance()->getItemCount("wood");
        auto& questData = questSystem->getQuestData(QuestType::COLLECT_WOOD);
        
        if (woodCount >= questData.targetAmount) {
            dialogueBox = DialogueBox::create(
                "Great! Thank you!",
                "Portraits/Lewis.png",
                "Lewis",
                lewis->getHeartPoints()
            );
            scene->addChild(dialogueBox, 10);
            if (uiService) {
                uiService->setDialogueBox(dialogueBox);
            }
            
            // 完成任务
            ItemSystem::getInstance()->removeItem("wood", questData.targetAmount);
            questSystem->completeQuest(QuestType::COLLECT_WOOD);
            
            // 移除任务提示UI
            if (uiService) {
                uiService->setQuestTipLabelVisible(false);
            }
        }
        else {
            dialogueBox = DialogueBox::create(
                "Go on, I need " + std::to_string(questData.targetAmount - woodCount) + " more.",
                "Portraits/Lewis.png",
                "Lewis",
                lewis->getHeartPoints()
            );
            scene->addChild(dialogueBox, 10);
            if (uiService) {
                uiService->setDialogueBox(dialogueBox);
            }
        }
    }
    else if (questState == QuestState::COMPLETED) {
        // 任务完成后显示随机对话
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        dialogueBox = DialogueBox::create(lewis->getRandomDialogue(), "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
        scene->addChild(dialogueBox, 10);
        if (uiService) {
            uiService->setDialogueBox(dialogueBox);
        }
    }
}

void EventService::handleBridgeQuest(Lewis* lewis, QuestState questState) {
    if (!lewis || !scene) return;
    
    auto questSystem = QuestSystem::getInstance();
    DialogueBox* dialogueBox = nullptr;
    
    if (questState == QuestState::NOT_STARTED) {
        // 显示任务介绍对话
        auto& questData = questSystem->getQuestData(QuestType::REPAIR_BRIDGE);
        dialogueBox = DialogueBox::create(questData.description, "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
        scene->addChild(dialogueBox, 10);
        if (uiService) {
            uiService->setDialogueBox(dialogueBox);
        }
        
        // 开始任务
        questSystem->startQuest(QuestType::REPAIR_BRIDGE);
        if (uiService) {
            uiService->hideQuestMark();
            uiService->createQuestTipLabelIfNeeded();
            uiService->updateQuestUI();
        }
    }
    else if (questState == QuestState::IN_PROGRESS) {
        dialogueBox = DialogueBox::create(
            "Come on! We will reach the other side of river.",
            "Portraits/Lewis.png",
            "Lewis",
            lewis->getHeartPoints()
        );
        scene->addChild(dialogueBox, 10);
        if (uiService) {
            uiService->setDialogueBox(dialogueBox);
        }
    }
    else if (questState == QuestState::COMPLETED) {
        static bool isFirstCompletion = true;
        
        if (isFirstCompletion) {
            // 首次完成显示特殊对话
            dialogueBox = DialogueBox::create(
                "Great job! Now we can get there!",
                "Portraits/Lewis.png",
                "Lewis",
                lewis->getHeartPoints()
            );
            isFirstCompletion = false;
            scene->addChild(dialogueBox, 10);
            if (uiService) {
                uiService->setDialogueBox(dialogueBox);
            }
        }
        else {
            // 后续显示随机对话
            std::srand(static_cast<unsigned int>(std::time(nullptr)));
            dialogueBox = DialogueBox::create(lewis->getRandomDialogue(), "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
            scene->addChild(dialogueBox, 10);
            if (uiService) {
                uiService->setDialogueBox(dialogueBox);
            }
        }
        
        // 移除任务提示UI
        if (uiService) {
            uiService->setQuestTipLabelVisible(false);
        }
    }
}

void EventService::checkQuestProgress() {
    auto questSystem = QuestSystem::getInstance();
    if (questSystem->getQuestState(QuestType::COLLECT_WOOD) == QuestState::IN_PROGRESS) {
        const int woodCount = ItemSystem::getInstance()->getItemCount("wood");
        questSystem->updateQuestProgress(QuestType::COLLECT_WOOD, woodCount);
        if (uiService) {
            uiService->updateQuestUI();
        }
    }
}

