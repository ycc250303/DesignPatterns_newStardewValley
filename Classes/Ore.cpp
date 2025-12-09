#include "Ore.h"
#include "GameMap.h"
#include "GameScene.h"
#include "ItemSystem.h"
#include "cocos2d.h"
#include <sstream>
#include <algorithm>

USING_NS_CC;

int Ore::nextId = 1;

Ore* Ore::create(const std::string& spriteName, int health)
{
    Ore* ore = new (std::nothrow) Ore();
    if (ore && ore->init(spriteName, health))
    {
        ore->autorelease();
        return ore;
    }
    CC_SAFE_DELETE(ore);
    return nullptr;
}

Ore::Ore() :
    _health(100),
    _dug(false),
    _canDug(true),
    _onOreDug(nullptr),
    entityId(nextId++)
{
}

Ore::~Ore()
{
}

bool Ore::init(const std::string& spriteName, int health)
{
    if (!Sprite::initWithFile(spriteName))
    {
        return false;
    }
    _health = health;
    _dug = false;
    _canDug = true;

    this->setAnchorPoint(Vec2(0.5f, 0.0f));
    this->setTextureRect(cocos2d::Rect(0, 0, 48, 48)); // ���ÿ�ʯ����������
    this->setScale(3.0f);
    return true;
}

void Ore::dig(int damage)
{
    if (!canBeDug())
    {
        return; // ��������ھ�ֱ�ӷ���
    }

    _health -= damage;

    // ʹ��Shake��ʵ�ֶ���Ч��
    auto shakeBy = Sequence::create(
        RotateBy::create(0.05f, -5),  // ������б5��
        RotateBy::create(0.05f, 10),   // ������б10��
        RotateBy::create(0.05f, -10),  // ������б10��
        RotateBy::create(0.05f, 5),    // �ص�ԭλ
        nullptr
    );
    this->runAction(shakeBy);

    if (_health <= 0)
    {
        _dug = true;

        // ��ʯ���ھ��Ķ���
        auto fadeOut = FadeOut::create(0.3f);
        auto removeOre = CallFunc::create([this]() {
            if (_onOreDug)
            {
                _onOreDug(); // �����ھ��Ļص�
            }
            this->removeFromParent(); // �Ƴ���ʯ
            });

        this->runAction(Sequence::create(
            fadeOut,
            removeOre,
            nullptr
        ));
    }
}

void Ore::remove()
{
    this->removeFromParentAndCleanup(true);
}

// 实现 GameEntity 接口
void Ore::initialize(const cocos2d::Vec2& tilePos, GameMap* map) {
    if (!map) return;
    Vec2 worldPos = map->convertToWorldCoord(tilePos);
    this->setPosition(worldPos);
    
    // 设置回调（需要GameScene引用，在EntityContainer中设置）
    if (_gameScene) {
        Vec2 ore_worldPos = worldPos;
        this->setOnOreDug([this, ore_worldPos]() {
            // 创建矿石掉落物
            auto stoneSprite = Sprite::create("TileSheets/Objects_2.png");
            if (stoneSprite && _gameScene) {
                stoneSprite->setTextureRect(Rect(65, 112, 16, 16));
                stoneSprite->setPosition(ore_worldPos);
                stoneSprite->setScale(1.5f);
                _gameScene->addChild(stoneSprite, 2);
                // 添加点击事件
                auto listener = EventListenerTouchOneByOne::create();
                listener->setSwallowTouches(true);
                listener->onTouchBegan = [=](Touch* touch, Event* event) {
                    Vec2 touchPos = touch->getLocation();
                    touchPos = _gameScene->convertToNodeSpace(touchPos);
                    if (stoneSprite->getBoundingBox().containsPoint(touchPos)) {
                        // 播放拾取动画
                        auto fadeOut = FadeOut::create(0.3f);
                        auto moveUp = MoveBy::create(0.3f, Vec2(0, 20));
                        auto spawn = Spawn::create(fadeOut, moveUp, nullptr);
                        auto remove = RemoveSelf::create();
                        // 添加矿石到物品系统
                        auto addItem = CallFunc::create([]() {
                            ItemSystem::getInstance()->addItem("stone", 10);
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
        });
    }
}

void Ore::update(float dt) {
    // Ore 不需要更新
}

void Ore::cleanup() {
    this->removeFromParent();
}

std::string Ore::getEntityId() const {
    std::ostringstream oss;
    oss << "ore_" << entityId;
    return oss.str();
}