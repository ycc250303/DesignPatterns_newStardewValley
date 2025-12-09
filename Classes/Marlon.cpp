#include "Marlon.h"
#include "GameMap.h"
#include "cocos2d.h"

USING_NS_CC;

Marlon* Marlon::create()
{
    Marlon* npc = new (std::nothrow) Marlon();
    if (npc && npc->init())
    {
        npc->autorelease();
        return npc;
    }
    CC_SAFE_DELETE(npc);
    return nullptr;
}

bool Marlon::init()
{
    const std::string imagePath = "Characters/Marlon.png"; // 马龙 NPC 的图片路径
    if (!Sprite::initWithFile(imagePath)) { // 直接使用 Sprite 的初始化
        return false;
    }
    //初始化基本信息
    moveSpeed = 50.0f;//移速
    heartPoint = 0;
    relationship = Relation::DEFAULT;

    //设置精灵属性
    this->setAnchorPoint(Vec2(0.5f, 0.5f));//设置锚点
    this->setScale(2.0f);//设置缩放

    this->setTextureRect(cocos2d::Rect(0, 0, 16, 32)); // 设置尺寸
    this->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f)); // 设置锚点
    this->setScale(2.5);
    initializeDefaultBehavior();
    return true;
}

void Marlon::initializeDefaultBehavior()
{
    setName("Malon");
    completeDialogue();
}

void Marlon::staticAnimation()
{
    ;
}

// 实现 GameEntity 接口
void Marlon::initialize(const cocos2d::Vec2& tilePos, GameMap* map) {
    if (!map) return;
    Vec2 worldPos = map->convertToWorldCoord(tilePos);
    this->setPosition(worldPos);
}

void Marlon::update(float dt) {
    // Marlon 不需要移动更新
}

void Marlon::cleanup() {
    this->removeFromParent();
}
