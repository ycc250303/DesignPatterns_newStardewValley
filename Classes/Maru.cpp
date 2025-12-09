#include "Maru.h"
#include "GameMap.h"
#include "cocos2d.h"

USING_NS_CC;

Maru* Maru::create()
{
    Maru* maru = new (std::nothrow) Maru();
    if (maru && maru->init())
    {
        maru->autorelease();
        return maru;
    }
    CC_SAFE_DELETE(maru);
    return nullptr;
}

bool Maru::init()
{
    const std::string imagePath = "Characters/Maru_Hospital.png"; // 玛鲁 NPC 的图片路径
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

void Maru::initializeDefaultBehavior()
{
    setName("Maru");
    completeDialogue();
}

void Maru::staticAnimation()
{
    ;
}

// 实现 GameEntity 接口
void Maru::initialize(const cocos2d::Vec2& tilePos, GameMap* map) {
    if (!map) return;
    Vec2 worldPos = map->convertToWorldCoord(tilePos);
    this->setPosition(worldPos);
}

void Maru::update(float dt) {
    // Maru 不需要移动更新
}

void Maru::cleanup() {
    this->removeFromParent();
}
