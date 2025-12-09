#include "Chicken.h"
#include "GameMap.h"
#include "cocos2d.h"
#include <sstream>

USING_NS_CC;

int Chicken::nextId = 1;

Chicken* Chicken::create()
{
	Chicken* chicken = new (std::nothrow) Chicken();
	if (chicken && chicken->init()) {
		chicken->autorelease();
		return chicken;
	}
	CC_SAFE_DELETE(chicken);
	return nullptr;
}

bool Chicken::init()
{
	const std::string imagePath = "Animals/White Chicken.png";
	if (!Sprite::initWithFile(imagePath)) {
		return false;
	}

	//初始化基本信息
	moveSpeed = 50.0f;
	heartPoint = 0;
	relationship = Relation::DEFAULT;
	entityId = nextId++;

	//设置精灵属性
	this->setAnchorPoint(Vec2(0.5f, 0.5f));
	this->setScale(2.0f);

	this->setTextureRect(cocos2d::Rect(0, 0, 16, 16));
	this->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->setScale(2.0);
	return true;
}

// 实现 GameEntity 接口
void Chicken::initialize(const cocos2d::Vec2& tilePos, GameMap* map) {
	if (!map) return;
	Vec2 worldPos = map->convertToWorldCoord(tilePos);
	this->setPosition(worldPos);
	staticAnimation();
}

void Chicken::initialize(const cocos2d::Vec2& tilePos, GameMap* map, const std::vector<cocos2d::Vec2>& path) {
	if (!map) return;
	Vec2 worldPos = map->convertToWorldCoord(tilePos);
	this->setPosition(worldPos);
	setPath(path);
}

void Chicken::update(float dt) {
	if (!path.empty()) {
		moveAlongPath(dt);
	}
}

void Chicken::cleanup() {
	this->removeFromParent();
}

std::string Chicken::getEntityId() const {
	std::ostringstream oss;
	oss << "chicken_" << entityId;
	return oss.str();
}

void Chicken::moveToDirection(cocos2d::Vec2& destination, float dt)
{
	cocos2d::Vec2 direction = destination - this->getPosition();
	float distance = direction.length();

	// ���·���
	if (direction.y > 0)
		currentDirection = 2;
	if (direction.y < 0)
		currentDirection = 0;
	if (direction.x < 0)
		currentDirection = 3;
	if (direction.x > 0)
		currentDirection = 1;

	// ��һ����������
	direction.normalize();
	// ����λ��
	this->setPosition(this->getPosition() + direction * moveSpeed * dt);
	// ������·����
	animationTimer += dt;
	if (animationTimer >= FRAME_INTERVAL) {
		animationTimer = 0;
		currentFrame = (currentFrame + 1) % 4;
	}
	// ������·֡
	if (currentDirection == 1) { // ����
		this->setScaleX(2.0f); // ȷ����������
		this->setTextureRect(cocos2d::Rect(currentFrame * 16, 16, 16, 16)); // ʹ�õڶ���
	}
	else if (currentDirection == 3) { // ����
		this->setScaleX(2.0f); // ȷ����������
		this->setTextureRect(cocos2d::Rect(currentFrame * 16, 48, 16, 16)); // ʹ�õ�����
	}
	else if (currentDirection == 2) { // ����
		this->setScaleX(2.0f); // ȷ����������
		this->setTextureRect(cocos2d::Rect(currentFrame * 16, 32, 16, 16)); // ʹ�õ�����
	}
	else if (currentDirection == 0) { // ����
		this->setScaleX(2.0f); // ȷ����������
		this->setTextureRect(cocos2d::Rect(currentFrame * 16, 0, 16, 16)); // ʹ�õ�һ��
	}

}

void Chicken::staticAnimation()
{
	currentActionState = ActionState::IDLE;
	// ��������
	auto animation = Animation::create();
	animation->setDelayPerUnit(1.0f); // ÿ֡����ʱ��

	// �������2֡����
	for (int i = 0; i < 2; i++) {
		auto frame = SpriteFrame::create("Animals/White Chicken.png", cocos2d::Rect(16 * i, 64, 16, 16)); // ÿ֡������
		animation->addSpriteFrame(frame);
	}

	// ����ѭ������
	auto runAnimation = RepeatForever::create(Animate::create(animation));
	this->runAction(runAnimation); // ���ж���
}

void Chicken::moveAlongPath(float dt)
{
	if (currentActionState != ActionState::MOVING) {
		return;
	}
	if (currentPathIndex >= path.size()) {
		currentPathIndex = 0;// �������·��ĩβ����������
	}

	cocos2d::Vec2 targetPosition = path[currentPathIndex];
	currentActionState = ActionState::MOVING;
	moveToDirection(targetPosition, dt);// �����ƶ�����

	// ����Ƿ񵽴�Ŀ��λ��
	if (this->getPosition().distance(targetPosition) < 1.0f) {
		//staticAnimation();// ���ž�ֹ����
		currentPathIndex++;// �ƶ�����һ��Ŀ��
	}
}