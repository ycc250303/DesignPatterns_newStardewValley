#include "EntityContainer.h"
#include "GameMap.h"
#include "GameScene.h"
#include "factory/NPCFactory.h"
#include "Lewis.h"
#include "Marlon.h"
#include "Maru.h"
#include "Alex.h"
#include "Pig.h"
#include "Chicken.h"
#include "Sheep.h"
#include "Tree.h"
#include "Ore.h"
#include "GameFacade.h"
#include "GameTime.h"
#include "QuestSystem.h"
#include "ItemSystem.h"
#include "cocos2d.h"
#include <algorithm>

USING_NS_CC;

EntityContainer::EntityContainer() {
}

EntityContainer::~EntityContainer() {
    clearEntities();
}

void EntityContainer::addEntity(std::shared_ptr<GameEntity> entity) {
    if (!entity) return;
    
    entities.push_back(entity);
    entitiesById[entity->getEntityId()] = entity;
    entitiesByType[entity->getEntityType()].push_back(entity);
}

void EntityContainer::removeEntity(const std::string& entityId) {
    auto it = entitiesById.find(entityId);
    if (it != entitiesById.end()) {
        auto entity = it->second;
        
        // 从entities中移除
        entities.erase(std::remove_if(entities.begin(), entities.end(),
            [&entity](const std::shared_ptr<GameEntity>& e) { return e == entity; }),
            entities.end());
        
        // 从entitiesByType中移除
        auto& typeEntities = entitiesByType[entity->getEntityType()];
        typeEntities.erase(std::remove_if(typeEntities.begin(), typeEntities.end(),
            [&entity](const std::shared_ptr<GameEntity>& e) { return e == entity; }),
            typeEntities.end());
        
        // 从entitiesById中移除
        entitiesById.erase(it);
    }
}

void EntityContainer::updateEntities(float dt) {
    for (auto& entity : entities) {
        if (entity) {
            entity->update(dt);
        }
    }
}

void EntityContainer::clearEntities() {
    for (auto& entity : entities) {
        if (entity) {
            entity->cleanup();
        }
    }
    entities.clear();
    entitiesById.clear();
    entitiesByType.clear();
}

void EntityContainer::spawnEntitiesForMap(const std::string& mapName, GameMap* map, GameScene* scene) {
    if (!map || !scene) return;
    
    createEntitiesForMap(mapName, map, scene);
}

void EntityContainer::createEntitiesForMap(const std::string& mapName, GameMap* map, GameScene* scene) {
    if (mapName == "Farm") {
        // 创建 Lewis
        auto lewis = std::shared_ptr<Lewis>(static_cast<Lewis*>(NPCFactory::create("lewis")));
        if (lewis) {
            lewis->initialize(Vec2(15, 16), map);
            scene->addChild(lewis.get(), 1);
            addEntity(lewis);
        }
        
        // 创建猪
        // 第一只猪
        auto pig1 = std::shared_ptr<Pig>(static_cast<Pig*>(NPCFactory::create("pig")));
        if (pig1) {
            Vec2 path1_tilePos1 = Vec2(11, 14);
            Vec2 path1_tilePos2 = Vec2(11, 18);
            Vec2 path1_worldPos1 = map->convertToWorldCoord(path1_tilePos1);
            Vec2 path1_worldPos2 = map->convertToWorldCoord(path1_tilePos2);
            std::vector<Vec2> pig1Path = { path1_worldPos1, path1_worldPos2 };
            pig1->initialize(Vec2(11, 16), map, pig1Path);
            scene->addChild(pig1.get(), 1);
            addEntity(pig1);
        }
        
        // 第二只猪
        auto pig2 = std::shared_ptr<Pig>(static_cast<Pig*>(NPCFactory::create("pig")));
        if (pig2) {
            Vec2 path2_tilePos1 = Vec2(6, 14);
            Vec2 path2_tilePos2 = Vec2(10, 14);
            Vec2 path2_worldPos1 = map->convertToWorldCoord(path2_tilePos1);
            Vec2 path2_worldPos2 = map->convertToWorldCoord(path2_tilePos2);
            std::vector<Vec2> pig2Path = { path2_worldPos1, path2_worldPos2 };
            pig2->initialize(Vec2(9, 14), map, pig2Path);
            scene->addChild(pig2.get(), 1);
            addEntity(pig2);
        }
        
        // 第三只猪
        auto pig3 = std::shared_ptr<Pig>(static_cast<Pig*>(NPCFactory::create("pig")));
        if (pig3) {
            Vec2 path3_tilePos1 = Vec2(14, 19);
            Vec2 path3_tilePos2 = Vec2(14, 17);
            Vec2 path3_worldPos1 = map->convertToWorldCoord(path3_tilePos1);
            Vec2 path3_worldPos2 = map->convertToWorldCoord(path3_tilePos2);
            std::vector<Vec2> pig3Path = { path3_worldPos1, path3_worldPos2 };
            pig3->initialize(Vec2(14, 18), map, pig3Path);
            scene->addChild(pig3.get(), 1);
            addEntity(pig3);
        }
        
        // 第四只猪（静止）
        auto pig4 = std::shared_ptr<Pig>(static_cast<Pig*>(NPCFactory::create("pig")));
        if (pig4) {
            pig4->initialize(Vec2(13, 13), map);
            scene->addChild(pig4.get(), 1);
            addEntity(pig4);
        }
        
        // 创建鸡
        // 第一只鸡
        auto chicken1 = std::shared_ptr<Chicken>(static_cast<Chicken*>(NPCFactory::create("chicken")));
        if (chicken1) {
            Vec2 ch1_path1_tilePos = Vec2(26, 16);
            Vec2 ch1_path2_tilePos = Vec2(30, 16);
            Vec2 ch1_path1_worldPos = map->convertToWorldCoord(ch1_path1_tilePos);
            Vec2 ch1_path2_worldPos = map->convertToWorldCoord(ch1_path2_tilePos);
            std::vector<Vec2> chicken1Path = { ch1_path1_worldPos, ch1_path2_worldPos };
            chicken1->initialize(Vec2(28, 16), map, chicken1Path);
            scene->addChild(chicken1.get(), 1);
            addEntity(chicken1);
        }
        
        // 第二只鸡（静止）
        auto chicken2 = std::shared_ptr<Chicken>(static_cast<Chicken*>(NPCFactory::create("chicken")));
        if (chicken2) {
            chicken2->initialize(Vec2(30, 13), map);
            scene->addChild(chicken2.get(), 1);
            addEntity(chicken2);
        }
        
        // 创建羊
        // 第一只羊
        auto sheep1 = std::shared_ptr<Sheep>(static_cast<Sheep*>(NPCFactory::create("sheep")));
        if (sheep1) {
            Vec2 sh1_path1_tilePos = Vec2(28, 17);
            Vec2 sh1_path2_tilePos = Vec2(28, 22);
            Vec2 sh1_path1_worldPos = map->convertToWorldCoord(sh1_path1_tilePos);
            Vec2 sh1_path2_worldPos = map->convertToWorldCoord(sh1_path2_tilePos);
            std::vector<Vec2> sheep1Path = { sh1_path1_worldPos, sh1_path2_worldPos };
            sheep1->initialize(Vec2(28, 20), map, sheep1Path);
            scene->addChild(sheep1.get(), 1);
            addEntity(sheep1);
        }
        
        // 第二只羊
        auto sheep2 = std::shared_ptr<Sheep>(static_cast<Sheep*>(NPCFactory::create("sheep")));
        if (sheep2) {
            Vec2 sh2_path1_tilePos = Vec2(23, 19);
            Vec2 sh2_path2_tilePos = Vec2(27, 19);
            Vec2 sh2_path1_worldPos = map->convertToWorldCoord(sh2_path1_tilePos);
            Vec2 sh2_path2_worldPos = map->convertToWorldCoord(sh2_path2_tilePos);
            std::vector<Vec2> sheep2Path = { sh2_path1_worldPos, sh2_path2_worldPos };
            sheep2->initialize(Vec2(26, 19), map, sheep2Path);
            scene->addChild(sheep2.get(), 1);
            addEntity(sheep2);
        }
        
        // 第三只羊（静止）
        auto sheep3 = std::shared_ptr<Sheep>(static_cast<Sheep*>(NPCFactory::create("sheep")));
        if (sheep3) {
            sheep3->initialize(Vec2(26, 23), map);
            scene->addChild(sheep3.get(), 1);
            addEntity(sheep3);
        }
        
        // 创建树
        auto tree1 = std::shared_ptr<Tree>(Tree::create("TileSheets/fruitTrees.png", 100));
        if (tree1) {
            tree1->setGameScene(scene);
            tree1->initialize(Vec2(22, 13), map);
            scene->addChild(tree1.get(), 1);
            addEntity(tree1);
        }
        
        auto tree2 = std::shared_ptr<Tree>(Tree::create("TileSheets/fruitTrees.png", 100));
        if (tree2) {
            tree2->setGameScene(scene);
            tree2->initialize(Vec2(36, 15), map);
            scene->addChild(tree2.get(), 1);
            addEntity(tree2);
        }
        
        auto tree3 = std::shared_ptr<Tree>(Tree::create("TileSheets/fruitTrees.png", 100));
        if (tree3) {
            tree3->setGameScene(scene);
            tree3->initialize(Vec2(35, 21), map);
            scene->addChild(tree3.get(), 1);
            addEntity(tree3);
        }
        
        // 创建任务UI
        GameFacade::instance().createQuestTipLabelIfNeeded();
    }
    else if (mapName == "Hospital") {
        // 创建 Maru
        auto maru = std::shared_ptr<Maru>(static_cast<Maru*>(NPCFactory::create("maru")));
        if (maru) {
            maru->initialize(Vec2(7, 14), map);
            scene->addChild(maru.get(), 1);
            addEntity(maru);
        }
    }
    else if (mapName == "Town") {
        // 创建 Alex
        auto alex = std::shared_ptr<Alex>(static_cast<Alex*>(NPCFactory::create("alex")));
        if (alex) {
            alex->initialize(Vec2(21, 20), map);
            scene->addChild(alex.get(), 1);
            addEntity(alex);
        }
    }
    else if (mapName == "Mine") {
        // 创建 Marlon
        auto marlon = std::shared_ptr<Marlon>(static_cast<Marlon*>(NPCFactory::create("marlon")));
        if (marlon) {
            marlon->initialize(Vec2(12, 10), map);
            scene->addChild(marlon.get(), 1);
            addEntity(marlon);
        }
        
        // 创建矿石
        // 矿洞右边
        for (int i = 0; i < 8; i++) {
            auto ore = std::shared_ptr<Ore>(Ore::create("TileSheets/Objects_2.png", 10));
            if (ore) {
                ore->setTextureRect(cocos2d::Rect(65, 112, 16, 16));
                ore->setGameScene(scene);
                ore->initialize(Vec2(17, 10 + i), map);
                scene->addChild(ore.get(), 1);
                addEntity(ore);
            }
        }
        
        // 矿洞左边
        for (int i = 0; i < 9; i++) {
            auto ore = std::shared_ptr<Ore>(Ore::create("TileSheets/Objects_2.png", 10));
            if (ore) {
                ore->setTextureRect(cocos2d::Rect(65, 112, 16, 16));
                ore->setGameScene(scene);
                ore->initialize(Vec2(3, 9 + i), map);
                scene->addChild(ore.get(), 1);
                addEntity(ore);
            }
        }
    }
}

std::vector<std::shared_ptr<GameEntity>> EntityContainer::getEntitiesByType(const std::string& type) {
    auto it = entitiesByType.find(type);
    if (it != entitiesByType.end()) {
        return it->second;
    }
    return std::vector<std::shared_ptr<GameEntity>>();
}

std::shared_ptr<GameEntity> EntityContainer::getEntityById(const std::string& entityId) {
    auto it = entitiesById.find(entityId);
    if (it != entitiesById.end()) {
        return it->second;
    }
    return nullptr;
}

