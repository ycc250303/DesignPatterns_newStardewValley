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
#include "service/UIService.h"
#include "service/EventService.h"
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
    isLewisCreated = false;     // 初始化标志
    isMarlonCreated = false;    // 初始化标志
    isMaruCreated = false;      // 初始化标志
    isAlexCreated = false;      // 初始化标志
    isMuted = false;            // 初始化为未静音
    // 初始化矿洞进入时间记录
    lastMineEnterDay = 0;
    lastMineEnterMonth = 0;
    lastMineEnterYear = 0;
    
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
    
    // 初始化鼠标监听器（用于钓鱼和实体交互）
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseDown = [this](Event* event)
        {
            auto fishingSystem = FishingSystem::getInstance();
            if (fishingSystem->isCurrentlyFishing()) {
                fishingSystem->finishFishing();
                return;
            }
            if (fishingSystem->canFish(player->getPosition(), player)) {
                fishingSystem->startFishing();
            }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
    
    // 初始化鼠标监听器（用于NPC交互、作物操作等）
    initMouseListener();
    
    // 初始化宝箱（实体初始化，保留在GameScene）
    initChests();
    
    // 启动更新
    this->scheduleUpdate();
    
    return true;
}
void GameScene::toggleMute(Ref* sender) {
    // 使用AudioService处理静音
    auto& facade = GameFacade::instance();
    facade.getAudioService()->toggleMute();
    isMuted = facade.getAudioService()->getIsMuted();
    
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
    
    // 更新Lewis的任务标记（实体相关，保留在GameScene）
    auto questSystem = QuestSystem::getInstance();
    auto gameTime = GameTime::getInstance();
    int newDay = gameTime->getDay();
    if (lewis) {
        auto uiService = GameFacade::instance().getUIService();
        if (newDay == 1 && questSystem->getQuestState(QuestType::COLLECT_WOOD) == QuestState::NOT_STARTED) {
            if (uiService) {
                uiService->showQuestMark(lewis);
            }
        } else if (newDay == 2 && questSystem->getQuestState(QuestType::REPAIR_BRIDGE) == QuestState::NOT_STARTED) {
            if (uiService) {
                uiService->showQuestMark(lewis);
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
    
    // 更新实体状态（保留在GameScene，因为依赖工厂模式）
    if (lewis) {
        lewis->moveAlongPath(dt);
    }
    for (auto pig : pigs) {
        if (pig) {
            pig->moveAlongPath(dt);
        }
    }
    for (auto chicken : chickens) {
        if (chicken) {
            chicken->moveAlongPath(dt);
        }
    }
    for (auto sheep : sheeps) {
        if (sheep) {
            sheep->moveAlongPath(dt);
        }
    }
    
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
    
    // 清理任务UI（通过UIService）
    auto uiService = GameFacade::instance().getUIService();
    if (uiService) {
        uiService->setQuestTipLabelVisible(false);
        uiService->hideQuestMark();
    }
    
    // 清理宝箱（实体初始化，保留在GameScene）
    clearChests();
    // 移除当前 NPC（实体初始化，保留在GameScene）
    if (lewis) {
        lewis->removeFromParent();
        lewis = nullptr;
    }
    if (marlon) {
        marlon->removeFromParent();
        marlon = nullptr;
    }
    if (maru) {
        maru->removeFromParent();
        maru = nullptr;
    }
    if (alex) {
        alex->removeFromParent();
        alex = nullptr;
    }
    // 移除所有动物（实体初始化，保留在GameScene）
    for (auto pig : pigs) {
        if (pig) pig->removeFromParent();
    }
    pigs.clear();
    for (auto chicken : chickens) {
        if (chicken) chicken->removeFromParent();
    }
    chickens.clear();
    for (auto sheep : sheeps) {
        if (sheep) sheep->removeFromParent();
    }
    sheeps.clear();
    // 隐藏所有树和矿石（实体初始化，保留在GameScene）
    for (auto tree : trees) {
        if (tree) tree->setVisible(false);
    }
    for (auto ore : ores) {
        if (ore) ore->setVisible(false);
    }
    
    // 获取当前时间
    auto gameTime = GameTime::getInstance();
    int currentMonth = gameTime->getMonth();
    int currentDay = gameTime->getDay();
    int currentYear = gameTime->getYear();
    
    // 根据地图名称初始化实体（实体初始化，保留在GameScene）
    if (mapName == "Farm") {
        CCLOG("Switching to Farm map, loading crops...");
        initLewis();
        initPig();
        initChicken();
        initSheep();
        initTree();
        for (auto tree : trees) {
            if (tree) tree->setVisible(true);
        }
        // 创建任务UI（如果需要）
        auto uiService = GameFacade::instance().getUIService();
        if (uiService) {
            uiService->createQuestTipLabelIfNeeded();
        }
    }
    else if (mapName == "Hospital") {
        CCLOG("Switching to Hospital map, initializing maru...");
        initMaru();
    }
    else if (mapName == "Town") {
        CCLOG("Switching to Town map, initializing alex...");
        initAlex();
    }
    else if (mapName == "Mine") {
        CCLOG("Switch to the mine map...");
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
        initMarlon();
        initOre();
        for (auto ore : ores) {
            if (ore) ore->setVisible(true);
        }
    }
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
            if (lewis) {
                const float distance = player->getPosition().distance(lewis->getPosition());
                if (distance < 50.0f) {
                    // 鼠标靠近Lewis，手上有礼物，变成礼物光标
                    if (player->getCurrentTool() == Player::ToolType::GIFT) {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_gift.png");
                    }
                    // 鼠标靠近Lewis，变成交互光标
                    else {
                        Director::getInstance()->getOpenGLView()->setCursor("cursor_dialogue.png");
                    }
                }
                else {
                    // 鼠标远离Lewis，恢复默认光标
                    Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
                }
            }
            if (marlon) {
                const float distance = player->getPosition().distance(marlon->getPosition());

                if (distance < 50.0f) {
                    Director::getInstance()->getOpenGLView()->setCursor("cursor_dialogue.png");
                }
                else {
                    Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
                }
            }
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
            if (alex) {
                const float distance = player->getPosition().distance(alex->getPosition());

                if (distance < 50.0f) {
                    Director::getInstance()->getOpenGLView()->setCursor("cursor_dialogue.png");
                }
                else {
                    Director::getInstance()->getOpenGLView()->setCursor("cursor_default.png");
                }
            }
        };

    mouseListener->onMouseDown = [=](Event* event)
        {
            const EventMouse* e = (EventMouse*)event;
            const Vec2 clickPos = e->getLocation(); // 获取点击位置

            // 鼠标点击时触发开垦
            if (player && player->getCurrentTool() == Player::ToolType::SHOVEL)
            {
                CropManager::getInstance()->onMouseDown(clickPos, player);
            }
            // 鼠标点击时触发浇水
            if (player && player->getCurrentTool() == Player::ToolType::WATERING)
            {
                CropManager::getInstance()->onMouseDown(clickPos, player);
            }
            // 鼠标点击时触发资源移除
            if (player && player->getCurrentTool() == Player::ToolType::AXE)
            {
                CropManager::getInstance()->onMouseDown(clickPos, player);
            }

            // 检查是否靠近并点击了刘易斯
            if (lewis) {
                const  float distance = player->getPosition().distance(lewis->getPosition());

                if (distance < 50.0f) {
                    player->setCanPerformAction(false); // 禁止玩家动作

                    // 使用EventService处理任务对话
                    auto eventService = GameFacade::instance().getEventService();
                    if (eventService) {
                        eventService->handleQuestDialogue(lewis);
                    }

                    // 处理礼物逻辑
                    if (player->getCurrentTool() == Player::ToolType::GIFT) {
                        lewis->stopAllActions();
                        lewis->showThanks();
                        dialogueBox = DialogueBox::create("I love this! Thank you! Mmmmmmm......", "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
                        this->addChild(dialogueBox, 10);

                        Size visibleSize = Director::getInstance()->getVisibleSize();

                        auto popup = cocos2d::Label::createWithSystemFont("Heartpoint with Lewis + 3", "Arial", 24);
                        popup->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 650);
                        popup->setAnchorPoint(Vec2(0.5f, 0.5f)); // 设置锚点为中心
                        popup->setColor(cocos2d::Color3B::BLACK);
                        this->addChild(popup, 100);

                        // 弹窗消失
                        auto fadeOutPopup = FadeOut::create(5.0f);
                        auto removePopup = RemoveSelf::create();
                        popup->runAction(Sequence::create(fadeOutPopup, removePopup, nullptr));

                        if (lewis->getHeartPoints() == 10) {
                            dialogueBox = DialogueBox::create("We are best friends!", "Portraits/Lewis.png", "Lewis", lewis->getHeartPoints());
                            this->addChild(dialogueBox, 10);
                        }
                    }
                    else {
                        lewis->staticAnimation(); // 静止状态
                    }
                }
                else {
                    player->setCanPerformAction(true);  // 允许玩家动作
                }
            }
            // 检查是否靠近并点击了马龙
            if (marlon) {
                const float distance = player->getPosition().distance(marlon->getPosition());

                if (distance < 50.0f) {
                    player->setCanPerformAction(false); // 禁止玩家动作
                    std::srand(static_cast<unsigned int>(std::time(nullptr)));

                    dialogueBox = DialogueBox::create(marlon->getRandomDialogue(), "Portraits/Marlon.png", "Marlon", marlon->getHeartPoints());
                    this->addChild(dialogueBox, 10);
                }
                else {
                    player->setCanPerformAction(true);  // 允许玩家动作
                }
            }
            // 检查是否靠近并点击了玛鲁
            if (maru) {
                const float distance = player->getPosition().distance(maru->getPosition());

                if (distance < 150.0f) {
                    player->setCanPerformAction(false); // 禁止玩家动作
                    std::srand(static_cast<unsigned int>(std::time(nullptr)));
                    dialogueBox = DialogueBox::create(maru->getRandomDialogue(), "Portraits/Maru_Hospital.png", "Maru", maru->getHeartPoints());
                    this->addChild(dialogueBox, 10);

                    // 处理礼物逻辑
                    if (player->getCurrentTool() == Player::ToolType::GIFT) {

                        // 好感度增加3
                        maru->heartPoint += 3;
                        dialogueBox = DialogueBox::create("Wow Yummy!", "Portraits/Maru.png", "Maru", maru->getHeartPoints());
                        this->addChild(dialogueBox, 10);
                        Size visibleSize = Director::getInstance()->getVisibleSize();

                        auto popup = cocos2d::Label::createWithSystemFont("Heartpoint with Maru + 3", "Arial", 24);
                        popup->setPosition(visibleSize.width / 2, visibleSize.height / 2 + 650);
                        popup->setAnchorPoint(Vec2(0.5f, 0.5f)); // 设置锚点为中心
                        popup->setColor(cocos2d::Color3B::BLACK);
                        this->addChild(popup, 100);

                        // 弹窗消失
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
                    player->setCanPerformAction(true);  // 允许玩家动作
                }
            }
            // 检查是否靠近并点击了艾利克斯
            if (alex) {
                float distance = player->getPosition().distance(alex->getPosition());

                if (distance < 50.0f) {
                    player->setCanPerformAction(false); // 禁止玩家动作
                    std::srand(static_cast<unsigned int>(std::time(nullptr)));

                    dialogueBox = DialogueBox::create(alex->getRandomDialogue(), "Portraits/Alex.png", "Alex", alex->getHeartPoints());
                    this->addChild(dialogueBox, 10);
                }
                else {
                    player->setCanPerformAction(true);  // 允许玩家动作
                }
            }
            // 检查是否靠近并点击了猪
            if (!pigs.empty()) {  // 确保vector不为空
                for (auto pig : pigs) {  // 遍历所有的pig
                    const float distance = player->getPosition().distance(pig->getPosition());
                    if (distance < 50.0f) {
                        if (player->getCurrentTool() == Player::ToolType::CARROT) {
                            // 如果玩家手持胡萝卜，触发吃饱了动画
                            pig->showFull();
                        }
                        break;
                    }
                }
            }
            // 检查是否靠近并点击了鸡
            if (!chickens.empty()) {  // 确保vector不为空
                for (auto chicken : chickens) {
                    const  float distance = player->getPosition().distance(chicken->getPosition());
                    if (distance < 50.0f) {
                        if (player->getCurrentTool() == Player::ToolType::CARROT) {
                            // 如果玩家手持胡萝卜，触发吃饱了动画
                            chicken->showFull();
                        }
                        break;
                    }
                }
            }
            // 检查是否靠近并点击了羊
            if (!sheeps.empty()) {  // 确保vector不为空
                for (auto sheep : sheeps) {
                    const float distance = player->getPosition().distance(sheep->getPosition());
                    if (distance < 50.0f) {
                        if (player->getCurrentTool() == Player::ToolType::CARROT) {
                            // 如果玩家手持胡萝卜，触发吃饱了动画
                            sheep->showFull();
                        }
                        break;
                    }
                }
            }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}
void GameScene::initLewis()
{
    lewis = Lewis::create();
    if (lewis == nullptr)
    {
        return;
    }
    // 设置刘易斯初始位置
    Vec2 lewis_tilePos = Vec2(15, 16);
    Vec2 lewis_worldPos = _gameMap->convertToWorldCoord(lewis_tilePos);
    lewis->setPosition(lewis_worldPos);
    this->addChild(lewis, 1);
    //设置刘易斯默认移动路径
    Vec2 movePath1_tilePos = Vec2(13, 16);
    Vec2 movePath2_tilePos = Vec2(18, 16);
    Vec2 movePath1_worldPos = _gameMap->convertToWorldCoord(movePath1_tilePos);
    Vec2 movePath2_worldPos = _gameMap->convertToWorldCoord(movePath2_tilePos);
    lewis->path.push_back(movePath1_worldPos);
    lewis->path.push_back(movePath2_worldPos);
    isLewisCreated = true; // 设置标志为已创建
    // 检查当前日期和任务状态
    auto gameTime = GameTime::getInstance();
    const  auto questSystem = QuestSystem::getInstance();
    const int currentDay = gameTime->getDay();
    // 第一天检查木头任务
    if (currentDay == 1 &&
        questSystem->getQuestState(QuestType::COLLECT_WOOD) == QuestState::NOT_STARTED) {
        showQuestMark(lewis);
    }
    // 第二天检查修桥任务
    else if (currentDay == 2 &&
        questSystem->getQuestState(QuestType::REPAIR_BRIDGE) == QuestState::NOT_STARTED) {
        showQuestMark(lewis);
    }
}
void GameScene::initMarlon()
{
    marlon = Marlon::create();
    if (marlon == nullptr) {
        return;
    }
    // 设置马龙初始位置
    const Vec2 marlon_tilePos = Vec2(12, 10);
    const Vec2 marlon_worldPos = _gameMap->convertToWorldCoord(marlon_tilePos);
    marlon->setPosition(marlon_worldPos);
    this->addChild(marlon, 1);
    isMarlonCreated = true; // 设置标志为已创建
}
void GameScene::initMaru()
{
    maru = Maru::create();
    if (maru == nullptr) {
        return;
    }
    // 设置玛鲁初始位置
    const Vec2 maru_tilePos = Vec2(7, 14);
    const Vec2 maru_worldPos = _gameMap->convertToWorldCoord(maru_tilePos);
    maru->setPosition(maru_worldPos);
    this->addChild(maru, 1);
    isMaruCreated = true; // 设置标志为已创建
}
void GameScene::initAlex()
{
    alex = Alex::create();
    if (alex == nullptr) {
        return;
    }
    // 设置艾利克斯初始位置
    const Vec2 alex_tilePos = Vec2(21, 20);
    const Vec2 alex_worldPos = _gameMap->convertToWorldCoord(alex_tilePos);
    alex->setPosition(alex_worldPos);
    this->addChild(alex, 1);
    alex->initializeAnimations();
    isAlexCreated = true; // 设置标志为已创建
}
void GameScene::createPig(const Vec2& initialPosition, const std::vector<Vec2>& path)
{
    Pig* newPig = Pig::create();                // 创建新的猪实例
    if (newPig) {
        newPig->setPosition(initialPosition);   // 设置初始位置
        // 设置猪初始位置
        const Vec2 newpig_tilePos = Vec2(initialPosition);
        const Vec2 newpig_worldPos = _gameMap->convertToWorldCoord(newpig_tilePos);
        newPig->setPosition(newpig_worldPos);
        newPig->setPath(path);                  // 设置路径
        this->addChild(newPig, 1);              // 将猪添加到场景中
        pigs.push_back(newPig);                 // 将新猪添加到向量中
    }
    else {
        return;
    }
}
void GameScene::createPig(const Vec2& initialPosition) // 重载 createPig 方法，用于创建静止猪
{
    Pig* staticPig = Pig::create();                     // 创建新的猪实例
    if (staticPig) {
        staticPig->setPosition(initialPosition);        // 设置初始位置
        // 设置猪初始位置
        const Vec2 staticpig_tilePos = Vec2(initialPosition);
        const Vec2 staticpig_worldPos = _gameMap->convertToWorldCoord(staticpig_tilePos);
        staticPig->setPosition(staticpig_worldPos);
        this->addChild(staticPig, 1);                   // 将静止猪添加到场景中
        pigs.push_back(staticPig);                      // 将静止猪添加到向量中
        staticPig->staticAnimation();                   // 播放静止动画
    }
    else {
        return;
    }
}
void GameScene::initPig()
{
    isPigCreated = true;                // 设置标志为已创建
    // 创建第一只猪
    const Vec2 movePig1Path1_tilePos = Vec2(11, 14);
    const Vec2 movePig1Path2_tilePos = Vec2(11, 18);
    const Vec2 movePig1Path1_worldPos = _gameMap->convertToWorldCoord(movePig1Path1_tilePos);
    const  Vec2 movePig1Path2_worldPos = _gameMap->convertToWorldCoord(movePig1Path2_tilePos);
    std::vector<Vec2> pig1Path = { movePig1Path1_worldPos,movePig1Path2_worldPos };
    createPig(Vec2(11, 16), pig1Path);  // 第一只猪的位置和路径
    // 创建第二只猪
    const Vec2 movePig2Path1_tilePos = Vec2(6, 14);
    const Vec2 movePig2Path2_tilePos = Vec2(10, 14);
    const Vec2 movePig2Path1_worldPos = _gameMap->convertToWorldCoord(movePig2Path1_tilePos);
    const Vec2 movePig2Path2_worldPos = _gameMap->convertToWorldCoord(movePig2Path2_tilePos);
    std::vector<Vec2> pig2Path = { movePig2Path1_worldPos,movePig2Path2_worldPos };
    createPig(Vec2(9, 14), pig2Path);   // 第二只猪的位置和路径
    // 创建第三只猪
    const Vec2 movePig3Path1_tilePos = Vec2(14, 19);
    const Vec2 movePig3Path2_tilePos = Vec2(14, 17);
    const Vec2 movePig3Path1_worldPos = _gameMap->convertToWorldCoord(movePig3Path1_tilePos);
    const Vec2 movePig3Path2_worldPos = _gameMap->convertToWorldCoord(movePig3Path2_tilePos);
    std::vector<Vec2> pig3Path = { movePig3Path1_worldPos,movePig3Path2_worldPos };
    createPig(Vec2(14, 18), pig3Path);  // 第三只猪的位置和路径
    // 创建第四只猪，保持静止
    createPig(Vec2(13, 13));            // 第四只猪的位置
}
void GameScene::createChicken(const Vec2& initialPosition, const std::vector<Vec2>& path)
{
    Chicken* newChicken = Chicken::create();            // 创建新的鸡实例
    if (newChicken) {
        newChicken->setPosition(initialPosition);       // 设置初始位置
        // 设置鸡初始位置
        const Vec2 newChicken_tilePos = Vec2(initialPosition);
        const Vec2 newChicken_worldPos = _gameMap->convertToWorldCoord(newChicken_tilePos);
        newChicken->setPosition(newChicken_worldPos);
        newChicken->setPath(path);                      // 设置路径
        this->addChild(newChicken, 1);                  // 将鸡添加到场景中
        chickens.push_back(newChicken);                 // 将新鸡添加到向量中
    }
    else {
        return;
    }
}
void GameScene::createChicken(const Vec2& initialPosition)  // 重载 createPig 方法，用于创建静止鸡
{
    Chicken* staticChicken = Chicken::create();             // 创建新的鸡实例
    if (staticChicken) {
        staticChicken->setPosition(initialPosition);        // 设置初始位置
        // 设置鸡初始位置
        const Vec2 staticChicken_tilePos = Vec2(initialPosition);
        const Vec2 staticChicken_worldPos = _gameMap->convertToWorldCoord(staticChicken_tilePos);
        staticChicken->setPosition(staticChicken_worldPos);
        this->addChild(staticChicken, 1);                   // 将静止鸡添加到场景中
        chickens.push_back(staticChicken);                  // 将静止鸡添加到向量中
        staticChicken->staticAnimation();                   // 播放静止动画
    }
    else {
        return;
    }
}
void GameScene::initChicken()
{
    isChickenCreated = true;                        // 设置标志为已创建
    // 创建第一只鸡
    const Vec2 moveChicken1Path1_tilePos = Vec2(26, 16);
    const Vec2 moveChicken1Path2_tilePos = Vec2(30, 16);
    const Vec2 moveChicken1Path1_worldPos = _gameMap->convertToWorldCoord(moveChicken1Path1_tilePos);
    const Vec2 moveChicken1Path2_worldPos = _gameMap->convertToWorldCoord(moveChicken1Path2_tilePos);
    std::vector<Vec2> Chicken1Path = { moveChicken1Path1_worldPos,moveChicken1Path2_worldPos };
    createChicken(Vec2(28, 16), Chicken1Path);      // 第一只鸡的位置和路径
    // 创建第二只鸡，保持静止
    createChicken(Vec2(30, 13));                    // 第二只鸡的位置
}
void GameScene::createSheep(const Vec2& initialPosition, const std::vector<Vec2>& path)
{
    Sheep* newSheep = Sheep::create();          // 创建新的羊实例
    if (newSheep) {
        newSheep->setPosition(initialPosition); // 设置初始位置
        // 设置羊初始位置
        const Vec2 newSheep_tilePos = Vec2(initialPosition);
        const Vec2 newSheep_worldPos = _gameMap->convertToWorldCoord(newSheep_tilePos);
        newSheep->setPosition(newSheep_worldPos);
        newSheep->setPath(path);                // 设置路径
        this->addChild(newSheep, 1);            // 将羊添加到场景中
        sheeps.push_back(newSheep);             // 将新羊添加到向量中
    }
    else {
        return;
    }
}
void GameScene::createSheep(const Vec2& initialPosition)  // 重载 createSheep 方法，用于创建静止羊
{
    Sheep* staticSheep = Sheep::create();           // 创建新的羊实例
    if (staticSheep) {
        staticSheep->setPosition(initialPosition);  // 设置初始位置
        // 设置羊初始位置
        const  Vec2 staticSheep_tilePos = Vec2(initialPosition);
        const Vec2 staticSheep_worldPos = _gameMap->convertToWorldCoord(staticSheep_tilePos);
        staticSheep->setPosition(staticSheep_worldPos);
        this->addChild(staticSheep, 1);             // 将静止羊添加到场景中
        sheeps.push_back(staticSheep);              // 将静止羊添加到向量中
        staticSheep->staticAnimation();             // 播放静止动画
    }
    else {
        return;
    }
}
void GameScene::initSheep()
{
    isSheepCreated = true;                  // 设置标志为已创建
    // 创建第一只羊
    const  Vec2 moveSheep1Path1_tilePos = Vec2(28, 17);
    const  Vec2 moveSheep1Path2_tilePos = Vec2(28, 22);
    const Vec2 moveSheep1Path1_worldPos = _gameMap->convertToWorldCoord(moveSheep1Path1_tilePos);
    const Vec2 moveSheep1Path2_worldPos = _gameMap->convertToWorldCoord(moveSheep1Path2_tilePos);
    std::vector<Vec2> sheep1Path = { moveSheep1Path1_worldPos,moveSheep1Path2_worldPos };
    createSheep(Vec2(28, 20), sheep1Path);  // 第一只羊的位置和路径
    // 创建第二只羊
    const  Vec2 moveSheep2Path1_tilePos = Vec2(23, 19);
    const Vec2 moveSheep2Path2_tilePos = Vec2(27, 19);
    const Vec2 moveSheep2Path1_worldPos = _gameMap->convertToWorldCoord(moveSheep2Path1_tilePos);
    const Vec2 moveSheep2Path2_worldPos = _gameMap->convertToWorldCoord(moveSheep2Path2_tilePos);
    std::vector<Vec2> sheep2Path = { moveSheep2Path1_worldPos,moveSheep2Path2_worldPos };
    createSheep(Vec2(26, 19), sheep2Path);  // 第二只羊的位置和路径
    // 创建第三只羊，保持静止
    createSheep(Vec2(26, 23));              // 第三只羊的位置
}
void GameScene::createTree(const Vec2& initialPosition)
{
    Tree* tree = Tree::create("TileSheets/fruitTrees.png", 100);
    if (tree) {
        const Vec2 tree_worldPos = _gameMap->convertToWorldCoord(initialPosition);
        tree->setPosition(tree_worldPos);
        // 设置树木被砍倒后的回调
        tree->setOnTreeChoppedCallback([this, tree, tree_worldPos]() {
            // 创建木头掉落物
            auto woodSprite = Sprite::create("TileSheets/fruitTrees.png");
            if (woodSprite) {
                woodSprite->setTextureRect(Rect(384, 700, 32, 32));
                woodSprite->setPosition(tree_worldPos);
                woodSprite->setScale(1.5f);
                this->addChild(woodSprite, 2);
                // 添加点击事件
                auto listener = EventListenerTouchOneByOne::create();
                listener->setSwallowTouches(true);
                listener->onTouchBegan = [this, woodSprite](Touch* touch, Event* event) {
                    Vec2 touchPos = touch->getLocation();
                    touchPos = this->convertToNodeSpace(touchPos);
                    if (woodSprite->getBoundingBox().containsPoint(touchPos)) {
                        // 播放拾取动画
                        auto fadeOut = FadeOut::create(0.3f);
                        auto moveUp = MoveBy::create(0.3f, Vec2(0, 20));
                        auto spawn = Spawn::create(fadeOut, moveUp, nullptr);
                        auto remove = RemoveSelf::create();
                        // 添加木头到物品系统
                        auto addItem = CallFunc::create([]() {
                            ItemSystem::getInstance()->addItem("wood", 10);
                            });
                        woodSprite->runAction(Sequence::create(
                            spawn,
                            addItem,
                            remove,
                            nullptr
                        ));
                        return true;
                    }
                    return false;
                    };
                Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, woodSprite);
                // 添加掉落动画
                const  auto dropDistance = 50.0f;
                const  auto dropDuration = 0.5f;
                woodSprite->setPositionY(woodSprite->getPositionY() + dropDistance);
                auto dropAction = EaseOut::create(
                    MoveBy::create(dropDuration, Vec2(0, -dropDistance)),
                    2.0f
                );
                woodSprite->runAction(dropAction);
            }
            // 移除树木
            tree->removeFromParent();
            auto iter = std::find(trees.begin(), trees.end(), tree);
            if (iter != trees.end()) {
                trees.erase(iter);
            }
            });
        this->addChild(tree, 1);
        trees.push_back(tree);
    }
}
void GameScene::initTree()
{
    if (!isTreeCreated) {
        createTree(Vec2(22, 13));
        createTree(Vec2(36, 15));
        createTree(Vec2(35, 21));
    }
    isTreeCreated = true; // 设置标志为已创建
}
void GameScene::createOre(const Vec2& initialPosition)
{
    Ore* ore = Ore::create("TileSheets/Objects_2.png", 10); // 假设矿石的初始生命值为10
    ore->setTextureRect(cocos2d::Rect(65, 112, 16, 16));
    if (ore) {
        Vec2 ore_worldPos = _gameMap->convertToWorldCoord(initialPosition);
        ore->setPosition(ore_worldPos);
        // 设置矿石被挖后的回调
        ore->setOnOreDug([this, ore, ore_worldPos]() {
            // 创建矿石掉落物
            auto stoneSprite = Sprite::create("TileSheets/Objects_2.png");
            if (stoneSprite) {
                stoneSprite->setTextureRect(Rect(65, 112, 16, 16)); // 矿石的纹理
                stoneSprite->setPosition(ore_worldPos);
                stoneSprite->setScale(1.5f);
                this->addChild(stoneSprite, 2);
                // 添加点击事件
                auto listener = EventListenerTouchOneByOne::create();
                listener->setSwallowTouches(true);
                listener->onTouchBegan = [this, stoneSprite](Touch* touch, Event* event) {
                    Vec2 touchPos = touch->getLocation();
                    touchPos = this->convertToNodeSpace(touchPos);
                    if (stoneSprite->getBoundingBox().containsPoint(touchPos)) {
                        // 播放拾取动画
                        auto fadeOut = FadeOut::create(0.3f);
                        auto moveUp = MoveBy::create(0.3f, Vec2(0, 20));
                        auto spawn = Spawn::create(fadeOut, moveUp, nullptr);
                        auto remove = RemoveSelf::create();
                        // 添加矿石到物品系统
                        auto addItem = CallFunc::create([]() {
                            ItemSystem::getInstance()->addItem("stone", 10); // 假设添加10个矿石
                            });
                        stoneSprite->runAction(Sequence::create(
                            spawn,
                            addItem,
                            remove,
                            nullptr
                        ));
                        return true;
                    }
                    return false;
                    };
                Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, stoneSprite);
                // 添加掉落动画
                auto dropDistance = 50.0f;
                auto dropDuration = 0.5f;
                stoneSprite->setPositionY(stoneSprite->getPositionY() + dropDistance);
                auto dropAction = EaseOut::create(
                    MoveBy::create(dropDuration, Vec2(0, -dropDistance)),
                    2.0f
                );
                stoneSprite->runAction(dropAction);
            }
            // 移除矿石
            ore->removeFromParent();
            auto iter = std::find(ores.begin(), ores.end(), ore);
            if (iter != ores.end()) {
                ores.erase(iter);
            }
            });
        this->addChild(ore, 1);
        ores.push_back(ore);
    }
}
void GameScene::initOre()
{
    if (!isOreCreated) {
        // 矿洞右边
        for (int i = 0; i < 8; i++) {
            createOre(Vec2(17, 10 + i));
        }
        // 矿洞左边
        for (int i = 0; i < 9; i++) {
            createOre(Vec2(3, 9 + i));
        }
    }
    isOreCreated = true; // 设置标志为已创建
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
    auto uiService = GameFacade::instance().getUIService();
    if (uiService) {
        uiService->showQuestMark(target);
    }
}
void GameScene::hideQuestMark(Node* target) {
    auto uiService = GameFacade::instance().getUIService();
    if (uiService) {
        uiService->hideQuestMark();
    }
}
// 任务相关方法已迁移到EventService和UIService
// 这些方法保留为空实现或转发调用，以防其他地方调用
void GameScene::updateQuestUI() {
    auto uiService = GameFacade::instance().getUIService();
    if (uiService) {
        uiService->updateQuestUI();
    }
}
void GameScene::handleQuestDialogue(Lewis* lewis) {
    auto eventService = GameFacade::instance().getEventService();
    if (eventService) {
        eventService->handleQuestDialogue(lewis);
    }
}
void GameScene::handleWoodQuest(Lewis* lewis, QuestState questState) {
    // 已迁移到EventService
}
void GameScene::handleBridgeQuest(Lewis* lewis, QuestState questState) {
    // 已迁移到EventService
}
void GameScene::updateQuestUIPosition() {
    auto uiService = GameFacade::instance().getUIService();
    if (uiService) {
        uiService->updateQuestUIPosition();
    }
}
void GameScene::checkQuestProgress() {
    auto eventService = GameFacade::instance().getEventService();
    if (eventService) {
        eventService->checkQuestProgress();
    }
}