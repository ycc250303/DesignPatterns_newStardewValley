#pragma once
#include "GameEntity.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

class GameMap;
class GameScene;

/**
 * 实体容器类（组合模式 - Composite）
 * 统一管理所有游戏实体
 */
class EntityContainer {
public:
    EntityContainer();
    ~EntityContainer();
    
    /**
     * 添加实体到容器
     * @param entity 实体指针
     */
    void addEntity(std::shared_ptr<GameEntity> entity);
    
    /**
     * 移除指定实体
     * @param entityId 实体ID
     */
    void removeEntity(const std::string& entityId);
    
    /**
     * 更新所有实体
     * @param dt 时间间隔
     */
    void updateEntities(float dt);
    
    /**
     * 清理所有实体
     */
    void clearEntities();
    
    /**
     * 根据地图名称生成实体
     * @param mapName 地图名称
     * @param map 游戏地图引用
     * @param scene 游戏场景引用
     */
    void spawnEntitiesForMap(const std::string& mapName, GameMap* map, GameScene* scene);
    
    /**
     * 根据类型获取实体列表
     * @param type 实体类型
     */
    std::vector<std::shared_ptr<GameEntity>> getEntitiesByType(const std::string& type);
    
    /**
     * 根据ID获取实体
     * @param entityId 实体ID
     */
    std::shared_ptr<GameEntity> getEntityById(const std::string& entityId);
    
    /**
     * 获取所有实体
     */
    const std::vector<std::shared_ptr<GameEntity>>& getAllEntities() const { return entities; }
    
private:
    std::vector<std::shared_ptr<GameEntity>> entities;
    std::unordered_map<std::string, std::shared_ptr<GameEntity>> entitiesById;
    std::unordered_map<std::string, std::vector<std::shared_ptr<GameEntity>>> entitiesByType;
    
    /**
     * 根据地图配置创建实体
     */
    void createEntitiesForMap(const std::string& mapName, GameMap* map, GameScene* scene);
};

