#include "factory/CropFactory.h"
#include "Corn.h"
#include "Tomato.h"
#include <unordered_map>

class CornCreator : public CropCreator {
public:
    Crop* create(const cocos2d::Vec2& worldPos) const override {
        return Corn::create(worldPos);
    }
};

class TomatoCreator : public CropCreator {
public:
    Crop* create(const cocos2d::Vec2& worldPos) const override {
        return Tomato::create(worldPos);
    }
};

/****************************************************************
 *
 * 使用工厂方法模式重构 - 重构后代码
 *
 ****************************************************************/
Crop* CropFactory::create(Player::SeedType type, const cocos2d::Vec2& worldPos) {
    static CornCreator cornCreator;
    static TomatoCreator tomatoCreator;
    static const std::unordered_map<Player::SeedType, CropCreator*> creators = {
        { Player::SeedType::CORN, &cornCreator },
        { Player::SeedType::TOMATO, &tomatoCreator }
    };

    auto it = creators.find(type);
    if (it != creators.end() && it->second) {
        return it->second->create(worldPos);
    }
    return nullptr;
}
