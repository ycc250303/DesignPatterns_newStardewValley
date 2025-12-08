#include "service/WeatherService.h"
#include "WeatherManager.h"
#include "Rain.h"
#include "Drought.h"
#include "NormalWeather.h"

void WeatherService::init() {
    // 默认设置为 Normal
    WeatherManager::getInstance()->setWeather(NormalWeather::create());
}

void WeatherService::update(float dt) {
    // 如需按时间刷新，可在此调用 randomRefreshWeather()
}

void WeatherService::randomRefreshWeather() {
    WeatherManager::getInstance()->randomRefreshWeather();
}

void WeatherService::setWeather(const std::string& type) {
    if (type == "rain") {
        WeatherManager::getInstance()->setWeather(Rain::create());
    } else if (type == "drought") {
        WeatherManager::getInstance()->setWeather(Drought::create());
    } else { // default normal
        WeatherManager::getInstance()->setWeather(NormalWeather::create());
    }
}