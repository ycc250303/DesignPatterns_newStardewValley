#pragma once
#include "Player.h"
#include "cocos2d.h"
class Crop;

class CropCreator {
public:
    virtual ~CropCreator() = default;
    virtual Crop* create(const cocos2d::Vec2& worldPos) const = 0;
};

class CropFactory {
public:
    static Crop* create(Player::SeedType type, const cocos2d::Vec2& worldPos);
};
