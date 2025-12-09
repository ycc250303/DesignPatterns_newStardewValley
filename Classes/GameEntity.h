#pragma once
#include "cocos2d.h"
#include <string>
#include <vector>

class GameMap;
class GameScene;

/**
 * 游戏实体抽象基类（组合模式 - Component）
 * 统一所有游戏实体的操作接口
 */
class GameEntity : public cocos2d::Node {
public:
    virtual ~GameEntity() = default;
    
    /**
     * 初始化实体
     * @param tilePos 初始位置（瓦片坐标）
     * @param map 游戏地图引用
     */
    virtual void initialize(const cocos2d::Vec2& tilePos, GameMap* map) = 0;
    
    /**
     * 初始化实体（带路径，用于动物）
     * @param tilePos 初始位置（瓦片坐标）
     * @param map 游戏地图引用
     * @param path 移动路径（世界坐标）
     */
    virtual void initialize(const cocos2d::Vec2& tilePos, GameMap* map, const std::vector<cocos2d::Vec2>& path) {
        // 默认实现，只调用无路径版本
        initialize(tilePos, map);
    }
    
    /**
     * 更新实体状态
     * @param dt 时间间隔
     */
    virtual void update(float dt) = 0;
    
    /**
     * 清理实体资源
     */
    virtual void cleanup() = 0;
    
    /**
     * 获取实体类型（如 "npc", "animal", "tree", "ore"）
     */
    virtual std::string getEntityType() const = 0;
    
    /**
     * 获取实体唯一ID
     */
    virtual std::string getEntityId() const = 0;
    
    /**
     * 判断实体是否应该在指定地图生成
     * @param mapName 地图名称
     */
    virtual bool shouldSpawnOnMap(const std::string& mapName) const = 0;
    
    /**
     * 设置实体是否可见
     */
    virtual void setVisible(bool visible) override = 0;
};

