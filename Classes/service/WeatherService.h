#pragma once
#include "cocos2d.h"

// 依赖现有的 WeatherManager / Weather / Rain / Drought / NormalWeather
class WeatherService {
public:
    void init();                 // 设置初始天气
    void update(float dt);       // 如需定时刷新天气，可在此实现
    void randomRefreshWeather(); // 外部可调用的随机刷新接口
    void setWeather(const std::string& type); // "rain"/"drought"/"normal"
};