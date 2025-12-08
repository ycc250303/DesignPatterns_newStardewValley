#pragma once
#include "cocos2d.h"

class AudioService {
public:
    void init();                       // 初始化时播放默认 BGM
    void update(float dt);             // 目前留空，如需做淡入淡出可在此实现
    void setMute(bool mute);           // 静音/恢复
    void toggleMute();                 // 切换静音状态
    void playBGM(const std::string& file);
    void pauseBGM();                    // 暂停BGM
    void setVolume(float volume);      // 0.0f ~ 1.0f
    
    // 根据地图和日期播放合适的BGM
    void playBGMForMap(const std::string& mapName, int month, int day);
    
    bool getIsMuted() const { return isMuted; }
private:
    bool isMuted = false;
    float cachedVolume = 1.0f;
};