#include "service/WeatherService.h"
#include "WeatherManager.h"
#include "factory/WeatherFactory.h"

void WeatherService::init() {
    // 默认设置为Normal
    WeatherManager::getInstance()->setWeather(WeatherFactory::create("normal"));
}

void WeatherService::update(float dt) {
    // 如需按时间刷新，可在此调度 randomRefreshWeather()
}

void WeatherService::randomRefreshWeather() {
    WeatherManager::getInstance()->randomRefreshWeather();
}

void WeatherService::setWeather(const std::string& type) {
/****************************************************************
 *
 * 使用工厂方法模式重构 - 重构后代码
 *
 ****************************************************************/
    WeatherManager::getInstance()->setWeather(WeatherFactory::create(type));
}

void WeatherService::onDayChanged(const DayChangedEvent&) {
    randomRefreshWeather();
}
