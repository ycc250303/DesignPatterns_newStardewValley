#include "Tree.h"
#include "GameMap.h"
#include "GameScene.h"
#include "ItemSystem.h"
#include "cocos2d.h"
#include <sstream>
#include <algorithm>

USING_NS_CC;

int Tree::nextId = 1;

Tree* Tree::create(const std::string& spriteName, int health)
{
    Tree* tree = new (std::nothrow) Tree();
    if (tree && tree->init(spriteName, health))
    {
        tree->autorelease();
        return tree;
    }
    CC_SAFE_DELETE(tree);
    return nullptr;
}

Tree::Tree() :
    _health(100),
    _chopped(false),
    _canChop(true),
    _onTreeChopped(nullptr),
    entityId(nextId++)
{
}

Tree::~Tree()
{
}

bool Tree::init(const std::string& spriteName, int health)
{
    if (!Sprite::initWithFile(spriteName))
    {
        return false;
    }
    _health = health;
    _chopped = false;
    _canChop = true;

    this->setAnchorPoint(Vec2(0.5f, 0.0f));
    this->setTextureRect(cocos2d::Rect(192, 160, 48, 80));
    this->setScale(3.0f);
    return true;
}

bool Tree::chop(int damage)
{
    if (!canBeChopped())
    {
        return false;
    }

    _health -= damage;

    // 使用RotateBy来实现抖动，这样会以底部为轴心抖动
    auto shakeBy = Sequence::create(
        RotateBy::create(0.05f, -5),  // 向左倾斜5度
        RotateBy::create(0.05f, 10),  // 向右倾斜10度
        RotateBy::create(0.05f, -10), // 向左倾斜10度
        RotateBy::create(0.05f, 5),   // 回到原位
        nullptr
    );
    this->runAction(shakeBy);

    if (_health <= 0)
    {
        _chopped = true;

        // 倒下动画也会以底部为轴心
        auto fallRotation = RotateTo::create(0.5f, 90.0f); // 向右倒下
        auto fadeOut = FadeOut::create(0.3f);
        auto removeTree = CallFunc::create([this]() {
            if (_onTreeChopped)
            {
                _onTreeChopped();
            }
            this->removeFromParent();
            });

        this->runAction(Sequence::create(
            fallRotation,
            fadeOut,
            removeTree,
            nullptr
        ));

        return true;
    }

    return false;
}

// 实现 GameEntity 接口
void Tree::initialize(const cocos2d::Vec2& tilePos, GameMap* map) {
    if (!map) return;
    Vec2 worldPos = map->convertToWorldCoord(tilePos);
    this->setPosition(worldPos);
    
    // 设置回调（需要GameScene引用，在EntityContainer中设置）
    if (_gameScene) {
        Vec2 tree_worldPos = worldPos;
        this->setOnTreeChoppedCallback([this, tree_worldPos]() {
            // 创建木头掉落物
            auto woodSprite = Sprite::create("TileSheets/fruitTrees.png");
            if (woodSprite && _gameScene) {
                woodSprite->setTextureRect(Rect(384, 700, 32, 32));
                woodSprite->setPosition(tree_worldPos);
                woodSprite->setScale(1.5f);
                _gameScene->addChild(woodSprite, 2);
                // 添加点击事件
                auto listener = EventListenerTouchOneByOne::create();
                listener->setSwallowTouches(true);
                listener->onTouchBegan = [=](Touch* touch, Event* event) {
                    Vec2 touchPos = touch->getLocation();
                    touchPos = _gameScene->convertToNodeSpace(touchPos);
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
                const auto dropDistance = 50.0f;
                const auto dropDuration = 0.5f;
                woodSprite->setPositionY(woodSprite->getPositionY() + dropDistance);
                auto dropAction = EaseOut::create(
                    MoveBy::create(dropDuration, Vec2(0, -dropDistance)),
                    2.0f
                );
                woodSprite->runAction(dropAction);
            }
        });
    }
}

void Tree::update(float dt) {
    // Tree 不需要更新
}

void Tree::cleanup() {
    this->removeFromParent();
}

std::string Tree::getEntityId() const {
    std::ostringstream oss;
    oss << "tree_" << entityId;
    return oss.str();
}

