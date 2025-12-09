#include "Pig.h"
#include "GameMap.h"
#include "cocos2d.h"
#include <sstream>

USING_NS_CC;

int Pig::nextId = 1;

Pig* Pig::create()
{
	Pig* pig = new (std::nothrow) Pig();
	if (pig && pig->init()) {
		pig->autorelease();
		return pig;
	}
	CC_SAFE_DELETE(pig);
	return nullptr;
}

bool Pig::init()
{
	const std::string imagePath = "Animals/Pig.png";
	if (!Sprite::initWithFile(imagePath)) { // 直接使用 Sprite 的初始化
		return false;
	}

	//初始化基本信息
	moveSpeed = 50.0f;//移速
	heartPoint = 0;
	relationship = Relation::DEFAULT;
	entityId = nextId++; // 分配唯一ID

	//设置精灵属性
	this->setAnchorPoint(Vec2(0.5f, 0.5f));//设置锚点
	this->setScale(2.0f);//设置缩放

	this->setTextureRect(cocos2d::Rect(0, 0, 32, 32));
	this->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->setScale(2.0);
	return true;
}

// 实现 GameEntity 接口
void Pig::initialize(const cocos2d::Vec2& tilePos, GameMap* map) {
	if (!map) return;
	Vec2 worldPos = map->convertToWorldCoord(tilePos);
	this->setPosition(worldPos);
	staticAnimation(); // 静止动画
}

void Pig::initialize(const cocos2d::Vec2& tilePos, GameMap* map, const std::vector<cocos2d::Vec2>& path) {
	if (!map) return;
	Vec2 worldPos = map->convertToWorldCoord(tilePos);
	this->setPosition(worldPos);
	setPath(path);
}

void Pig::update(float dt) {
	if (!path.empty()) {
		moveAlongPath(dt);
	}
}

void Pig::cleanup() {
	this->removeFromParent();
}

std::string Pig::getEntityId() const {
	std::ostringstream oss;
	oss << "pig_" << entityId;
	return oss.str();
}

void Pig::moveToDirection(cocos2d::Vec2& destination, float dt)
{
	cocos2d::Vec2 direction = destination - this->getPosition();
	float distance = direction.length();

	// 更新方向
	if (direction.y > 0)
		currentDirection = 2;
	if (direction.y < 0)
		currentDirection = 0;
	if (direction.x < 0)
		currentDirection = 3;
	if (direction.x > 0)
		currentDirection = 1;

	// 归一化方向向量
	direction.normalize();
	// 更新位置
	this->setPosition(this->getPosition() + direction * moveSpeed * dt);
	// 更新走路动画
	animationTimer += dt;
	if (animationTimer >= FRAME_INTERVAL) {
		animationTimer = 0;
		currentFrame = (currentFrame + 1) % 4;
	}
	// 设置走路帧
	if (currentDirection == 1) { // 向右
		this->setScaleX(2.0f); // 确保正常缩放
		this->setTextureRect(cocos2d::Rect(currentFrame * 32, 32, 32, 32)); // 使用第二行
	}
	else if (currentDirection == 3) { // 向左
		this->setScaleX(-2.0f); // 反转
		this->setTextureRect(cocos2d::Rect(currentFrame * 32, 32, 32, 32)); // 使用第二行反向
	}
	else if (currentDirection == 2) { // 向上
		this->setScaleX(2.0f); // 确保正常缩放
		this->setTextureRect(cocos2d::Rect(currentFrame * 32, 64, 32, 32)); // 使用第三行
	}
	else if (currentDirection == 0) { // 向下
		this->setScaleX(2.0f); // 确保正常缩放
		this->setTextureRect(cocos2d::Rect(currentFrame * 32, 0, 32, 32)); // 使用第一行
	}

}

void Pig::staticAnimation()
{
	currentActionState = ActionState::IDLE;
	// 创建动画
	auto animation = Animation::create();
	animation->setDelayPerUnit(1.0f); // 每帧持续时间

	// 精灵表有4帧动画
	for (int i = 0; i < 4; i++) {
		auto frame = SpriteFrame::create("Animals/Pig.png", cocos2d::Rect(32 * i, 128, 32, 32)); // 每帧的区域
		animation->addSpriteFrame(frame);
	}

	// 创建循环动画
	auto runAnimation = RepeatForever::create(Animate::create(animation));
	this->runAction(runAnimation); // 运行动画
}

void Pig::moveAlongPath(float dt)
{
	if (currentActionState != ActionState::MOVING) {
		return;
	}
	if (currentPathIndex >= path.size()) {
		currentPathIndex = 0;// 如果到达路径末尾，重置索引
	}

	cocos2d::Vec2 targetPosition = path[currentPathIndex];
	currentActionState = ActionState::MOVING;
	moveToDirection(targetPosition, dt);// 调用移动方法

	// 检测是否到达目标位置
	if (this->getPosition().distance(targetPosition) < 1.0f) {
		//staticAnimation();// 播放静止动画
		currentPathIndex++;// 移动到下一个目标
	}
}
