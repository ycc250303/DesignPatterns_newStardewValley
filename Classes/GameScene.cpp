#include "GameScene.h"
#include "GameFacade.h"
#include "GameTime.h"
#include "LightManager.h"
#include "WeatherManager.h"
#include "Chest.h" 
#include "Sleep.h"
#include "BridgeEvent.h"
#include "Cooking.h"
#include "CropManager.h"
#include "NormalWeather.h"
#include "AudioManager.h"
#include "FishingSystem.h"
#include "EntityContainer.h"
#include "Lewis.h"
#include "Marlon.h"
#include "Maru.h"
#include "Alex.h"
#include "Pig.h"
#include "Chicken.h"
#include "Sheep.h"
#include "QuestSystem.h"
#include "DialogueBox.h"
#include <memory>
#include <ctime>
#include <cstdlib>
USING_NS_CC;
// UI 
Scene* GameScene::createScene()
{
    return GameScene::create();
}

bool GameScene::init()
{
    if (!Scene::init())
        return false;
    // 初始化变量
    player = nullptr;
    _gameMap = nullptr;
    _pressedKeys.clear();
    isMuted = false;            // 初始化为未静音
    // 初始化矿洞进入时间记录
    lastMineEnterDay = 0;
    lastMineEnterMonth = 0;
    lastMineEnterYear = 0;
    
    // 创建实体容器（组合模式）
    entityContainer = std::make_unique<EntityContainer>();
    
    // 使用GameFacade初始化
    GameFacade::instance().initialize(this);
    
    // 获取Facade管理的对象引用
    player = GameFacade::instance().getPlayer();
    _gameMap = GameFacade::instance().getMap();
    
    if (!player || !_gameMap) {
        return false;
    }
    
    // 设置CropManager的地图引用和场景引用
    auto cropManager = CropManager::getInstance();
    cropManager->setGameScene(this);
    cropManager->setGameMap(_gameMap);
    
    // 初始化钓鱼区域
    FishingSystem::getInstance()->initFishingAreas(_gameMap);
    
    // 初始化鼠标监听器（用于NPC交互、作物操作等）
    initMouseListener();
    
    // 初始化宝箱（实体初始化，保留在GameScene）
    initChests();
    
    // 启动更新
    this->scheduleUpdate();
    
    return true;
}
void GameScene::toggleMute(Ref* sender) {
    // 使用GameFacade处理静音（符合外观模式）
    auto& facade = GameFacade::instance();
    facade.toggleMute();
    isMuted = facade.getIsMuted();
    
    // 更新按钮图标
    if (muteButton) {
        if (isMuted) {
            muteButton->setNormalImage(Sprite::create("unmute.png"));
        } else {
            muteButton->setNormalImage(Sprite::create("mute.png"));
        }
    }
}
// update
void GameScene::onDayChanged()
{
    // 使用GameFacade处理日期变化
    GameFacade::instance().onDayChanged();
    
    // 更新Lewis的任务标记（通过EntityContainer获取）
    auto questSystem = QuestSystem::getInstance();
    auto gameTime = GameTime::getInstance();
    int newDay = gameTime->getDay();
    auto lewisEntity = entityContainer->getEntityById("lewis");
    if (lewisEntity) {
        auto lewis = dynamic_cast<Lewis*>(lewisEntity.get());
        if (lewis) {
            auto& facade = GameFacade::instance();
            if (newDay == 1 && questSystem->getQuestState(QuestType::COLLECT_WOOD) == QuestState::NOT_STARTED) {
                facade.showQuestMark(lewis);
            } else if (newDay == 2 && questSystem->getQuestState(QuestType::REPAIR_BRIDGE) == QuestState::NOT_STARTED) {
                facade.showQuestMark(lewis);
            }
        }
    }
}
void GameScene::update(float dt)
{
    if (!player || !_gameMap) {
        return;
    }
    
    // 使用GameFacade更新所有核心逻辑
    GameFacade::instance().update(dt);
    
    // 检查传送点
    std::string targetMap;
    Vec2 targetTilePos;
    if (GameFacade::instance().checkTransition(targetMap, targetTilePos)) {
        switchToMap(targetMap, targetTilePos);
    }
    
    // 统一更新所有实体（组合模式）
    entityContainer->updateEntities(dt);
    
    // 更新钓鱼提示标签位置
    auto fishingSystem = FishingSystem::getInstance();
    fishingSystem->canFish(player->getPosition(), player);
    if (auto tipLabel = fishingSystem->getTipLabel()) {
        if (tipLabel->isVisible()) {
            Vec2 playerPos = player->getPosition();
            tipLabel->setPosition(playerPos + Vec2(0, 50));
        }
    }
    
    // 更新作物提示和杀虫状态
    Vec2 playerTilePos = _gameMap->convertToTileCoord(player->getPosition());
    CropManager::getInstance()->updateTips(playerTilePos, player->getCurrentTool());
    CropManager::getInstance()->updateBugKilling(dt);
}
void GameScene::updateCamera()
{
    // 相机更新已迁移到UIService::updateCameraAndUI
    // 此方法保留为空实现，以防其他地方调用
}
void GameScene::switchToMap(const std::string& mapName, const cocos2d::Vec2& targetTilePos)
{
    // 使用GameFacade切换地图（核心逻辑）
    GameFacade::instance().switchMap(mapName, targetTilePos);
    
    // 更新本地引用
    player = GameFacade::instance().getPlayer();
    _gameMap = GameFacade::instance().getMap();
    
    if (!player || !_gameMap) {
        return;
    }
    
    // 清理任务UI（通过GameFacade，符合外观模式）
    auto& facade = GameFacade::instance();
    facade.setQuestTipLabelVisible(false);
    facade.hideQuestMark();
    
    // 清理宝箱（实体初始化，保留在GameScene）
    clearChests();
    
    // 清理所有实体（组合模式 - 统一管理）
    entityContainer->clearEntities();
    
    // 获取当前时间
    auto gameTime = GameTime::getInstance();
    int currentMonth = gameTime->getMonth();
    int currentDay = gameTime->getDay();
    int currentYear = gameTime->getYear();
    
    // 根据地图名称生成实体（组合模式 - 统一管理）
    if (mapName == "Mine") {
        // 检查是否需要刷新宝箱
        bool shouldRefreshChests = false;
        if (lastMineEnterDay == 0) {
            shouldRefreshChests = true;
        }
        else if (currentYear > lastMineEnterYear ||
            (currentYear == lastMineEnterYear && currentMonth > lastMineEnterMonth) ||
            (currentYear == lastMineEnterYear && currentMonth == lastMineEnterMonth && currentDay > lastMineEnterDay)) {
            shouldRefreshChests = true;
        }
        if (shouldRefreshChests) {
            CCLOG("Refresh the mine treasure chest...");
            initChests();
            lastMineEnterDay = currentDay;
            lastMineEnterMonth = currentMonth;
            lastMineEnterYear = currentYear;
        }
    }
    
    // 统一生成实体（组合模式）
    entityContainer->spawnEntitiesForMap(mapName, _gameMap, this);
}
// 初始化
void GameScene::initMouseListener()
{
    auto mouseListener = EventListenerMouse::create();
    // 设置默认光标
    Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
    mouseListener->onMouseMove = [=](Event* event)
        {
            EventMouse* e = (EventMouse*)event;
            const Vec2 mousePos = e->getLocation();
            // 通过EntityContainer获取NPC（组合模式）
            auto lewisEntity = entityContainer->getEntityById("lewis");
            if (lewisEntity) {
                auto lewis = dynamic_cast<Lewis*>(lewisEntity.get());
                if (lewis) {
                    const float distance = player->getPosition().distance(lewis->getPosition());
                    if (distance < 50.0f) {
                        if (player->getCurrentTool() == Player::ToolType::GIFT) {
                            Director::getInstance()->getOpenGLView()->setCursor("cursor_gift.png");
                        }
                        else {
                            Director::getInstance()->getOpenGLView()->setCursor("cursor_dialogue.png");
                        }
                    }
                    else {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
                    }
                }
            }
            
            auto marlonEntity = entityContainer->getEntityById("marlon");
            if (marlonEntity) {
                auto marlon = dynamic_cast<Marlon*>(marlonEntity.get());
                if (marlon) {
                    const float distance = player->getPosition().distance(marlon->getPosition());
                    if (distance < 50.0f) {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_dialogue.png");
                    }
                    else {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
                    }
                }
            }
            
            auto maruEntity = entityContainer->getEntityById("maru");
            if (maruEntity) {
                auto maru = dynamic_cast<Maru*>(maruEntity.get());
                if (maru) {
                    const float distance = player->getPosition().distance(maru->getPosition());
                    if (distance < 120.0f) {
                        if (player->getCurrentTool() == Player::ToolType::GIFT) {
                            Director::getInstance()->getOpenGLView()->setCursor("cursor_gift.png");
                        }
                        else {
                            Director::getInstance()->getOpenGLView()->setCursor("cursor_dialogue.png");
                        }
                    }
                    else {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
                    }
                }
            }
            
            auto alexEntity = entityContainer->getEntityById("alex");
            if (alexEntity) {
                auto alex = dynamic_cast<Alex*>(alexEntity.get());
                if (alex) {
                    const float distance = player->getPosition().distance(alex->getPosition());
                    if (distance < 50.0f) {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_dialogue.png");
                    }
                    else {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
                    }
                }
            }
        };

    mouseListener->onMouseDown = [=](Event* event)
        {
            const EventMouse* e = (EventMouse*)event;
            const Vec2 clickPos = e->getLocation(); // 获取点击位置

            // 检查是否靠近并点击了刘易斯（通过EntityContainer获取）
            auto lewisEntity = entityContainer->getEntityById("lewis");
            if (lewisEntity) {
                auto lewis = dynamic_cast<Lewis*>(lewisEntity.get());
                if (lewis) {
                    const float distance = player->getPosition().distance(lewis->getPosition());
                    if (distance < 50.0f) {
                        player->setCanPerformAction(false);
                        GameFacade::instance().handleQuestDialogue(lewis);
                        if (player->getCurrentTool() == Player::ToolType::GIFT) {
                            lewis->stopAllActions();
                            lewis->showThanks();
                            dialogueBox = DialogueBox::create("I love this! Thank you! Mmmmmmm......", "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
                            this->addChild(dialogueBox, 10);
                            Size visibleSize = Director::getInstance()->getVisibleSize();
                            auto popup = cocos2d::Label::createWithSystemFont("Heartpoint with Lewis + 3", "Arial", 24);
                            popup->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 650);
                            popup->setAnchorPoint(Vec2(0.5f, 0.5f));
                            popup->setColor(cocos2d::Color3B::BLACK);
                            this->addChild(popup, 100);
                            auto fadeOutPopup = FadeOut::create(5.0f);
                            auto removePopup = RemoveSelf::create();
                            popup->runAction(Sequence::create(fadeOutPopup, removePopup, nullptr));
                            if (lewis->getHeartPoints() == 10) {
                                dialogueBox = DialogueBox::create("We are best friends!", "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
                                this->addChild(dialogueBox, 10);
                            }
                        }
                        else {
                            lewis->staticAnimation();
                        }
                    }
                    else {
                        player->setCanPerformAction(true);
                    }
                }
            }
            
            // 检查是否靠近并点击了马龙
            auto marlonEntity = entityContainer->getEntityById("marlon");
            if (marlonEntity) {
                auto marlon = dynamic_cast<Marlon*>(marlonEntity.get());
                if (marlon) {
                    const float distance = player->getPosition().distance(marlon->getPosition());
                    if (distance < 50.0f) {
                        player->setCanPerformAction(false);
                        std::srand(static_cast<unsigned int>(std::time(nullptr)));
                        dialogueBox = DialogueBox::create(marlon->getRandomDialogue(), "Portraits/Marlon.png", "Marlon", marlon->getHeartPoints());
                        this->addChild(dialogueBox, 10);
                    }
                    else {
                        player->setCanPerformAction(true);
                    }
                }
            }
            
            // 检查是否靠近并点击了玛鲁
            auto maruEntity = entityContainer->getEntityById("maru");
            if (maruEntity) {
                auto maru = dynamic_cast<Maru*>(maruEntity.get());
                if (maru) {
                    const float distance = player->getPosition().distance(maru->getPosition());
                    if (distance < 150.0f) {
                        player->setCanPerformAction(false);
                        std::srand(static_cast<unsigned int>(std::time(nullptr)));
                        dialogueBox = DialogueBox::create(maru->getRandomDialogue(), "Portraits/Maru_Hospital.png", "Maru", maru->getHeartPoints());
                        this->addChild(dialogueBox, 10);
                        if (player->getCurrentTool() == Player::ToolType::GIFT) {
                            maru->heartPoint += 3;
                            dialogueBox = DialogueBox::create("Wow Yummy!", "Portraits/Maru.png", "Maru", maru->getHeartPoints());
                            this->addChild(dialogueBox, 10);
                            Size visibleSize = Director::getInstance()->getVisibleSize();
                            auto popup = cocos2d::Label::createWithSystemFont("Heartpoint with Maru + 3", "Arial", 24);
                            popup->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 650);
                            popup->setAnchorPoint(Vec2(0.5f, 0.5f));
                            popup->setColor(cocos2d::Color3B::BLACK);
                            this->addChild(popup, 100);
                            auto fadeOutPopup = FadeOut::create(5.0f);
                            auto removePopup = RemoveSelf::create();
                            popup->runAction(Sequence::create(fadeOutPopup, removePopup, nullptr));
                            if (maru->getHeartPoints() == 10) {
                                dialogueBox = DialogueBox::create("I love you forever!!!", "Portraits/Maru.png", "Maru", maru->getHeartPoints());
                                this->addChild(dialogueBox, 10);
                            }
                        }
                        else {
                            maru->staticAnimation();
                        }
                    }
                    else {
                        player->setCanPerformAction(true);
                    }
                }
            }
            
            // 检查是否靠近并点击了艾利克斯
            auto alexEntity = entityContainer->getEntityById("alex");
            if (alexEntity) {
                auto alex = dynamic_cast<Alex*>(alexEntity.get());
                if (alex) {
                    float distance = player->getPosition().distance(alex->getPosition());
                    if (distance < 50.0f) {
                        player->setCanPerformAction(false);
                        std::srand(static_cast<unsigned int>(std::time(nullptr)));
                        dialogueBox = DialogueBox::create(alex->getRandomDialogue(), "Portraits/Alex.png", "Alex", alex->getHeartPoints());
                        this->addChild(dialogueBox, 10);
                    }
                    else {
                        player->setCanPerformAction(true);
                    }
                }
            }
            
            // 检查是否靠近并点击了动物（通过EntityContainer获取）
            auto animals = entityContainer->getEntitiesByType("animal");
            for (auto& animalEntity : animals) {
                if (auto pig = dynamic_cast<Pig*>(animalEntity.get())) {
                    const float distance = player->getPosition().distance(pig->getPosition());
                    if (distance < 50.0f && player->getCurrentTool() == Player::ToolType::CARROT) {
                        pig->showFull();
                        break;
                    }
                }
                else if (auto chicken = dynamic_cast<Chicken*>(animalEntity.get())) {
                    const float distance = player->getPosition().distance(chicken->getPosition());
                    if (distance < 50.0f && player->getCurrentTool() == Player::ToolType::CARROT) {
                        chicken->showFull();
                        break;
                    }
                }
                else if (auto sheep = dynamic_cast<Sheep*>(animalEntity.get())) {
                    const float distance = player->getPosition().distance(sheep->getPosition());
                    if (distance < 50.0f && player->getCurrentTool() == Player::ToolType::CARROT) {
                        sheep->showFull();
                        break;
                    }
                }
            }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

InventoryUI* GameScene::getInventoryUI() const {
    // 通过GameFacade获取InventoryUI（符合外观模式）
    return GameFacade::instance().getInventoryUI();
}

void GameScene::initChests()
{
    if (!_gameMap) {
        return;
    }
    auto objectGroup = _gameMap->getTileMap()->getObjectGroup("Chest");
    if (!objectGroup) {
        return;
    }
    // 获取所有宝箱对象
    auto& objects = objectGroup->getObjects();
    for (auto& obj : objects) {
        auto& dict = obj.asValueMap();
        const float x = dict["x"].asFloat();
        const  float y = dict["y"].asFloat();
        // 创建宝箱
        auto chest = Chest::create();
        if (chest) {
            // 获取地图的瓦片大小和缩放比例
            const Size tileSize = _gameMap->getTileMap()->getTileSize();
            const  float scale = _gameMap->getTileMap()->getScale();
            // 将 Tiled 对象坐标转换为瓦片坐标
            const float tileX = x / tileSize.width;
            const float tileY = y / tileSize.height;
            // 使用 GameMap 的转换方法获取世界坐标
            const Vec2 worldPos = _gameMap->convertToWorldCoord(Vec2(tileX, tileY));
            chest->setPosition(worldPos);
            chest->setScale(2.0f);
            chest->initTouchEvents();
            // 添加到场景
            this->addChild(chest, 1);
            _chests.push_back(chest);
        }
    }
}
void GameScene::clearChests()
{
    for (auto chest : _chests) {
        if (chest) {
            chest->removeFromParent();
        }
    }
    _chests.clear();
}
// 任务显示 
void GameScene::showQuestMark(Node* target) {
    GameFacade::instance().showQuestMark(target);
}

void GameScene::hideQuestMark(Node* target) {
    GameFacade::instance().hideQuestMark();
}

// 任务相关方法已迁移到EventService和UIService
// 这些方法保留为转发调用，通过GameFacade访问（符合外观模式）
void GameScene::updateQuestUI() {
    GameFacade::instance().updateQuestUI();
}

void GameScene::handleQuestDialogue(Lewis* lewis) {
    GameFacade::instance().handleQuestDialogue(lewis);
}

void GameScene::updateQuestUIPosition() {
    GameFacade::instance().updateQuestUIPosition();
}

void GameScene::checkQuestProgress() {
    GameFacade::instance().checkQuestProgress();
}
