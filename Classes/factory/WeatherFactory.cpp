#include "factory/WeatherFactory.h"
#include "Rain.h"
#include "Drought.h"
#include "NormalWeather.h"
#include <unordered_map>
#include <string>

class RainCreator : public WeatherCreator {
public:
    Weather* create() const override { return Rain::create(); }
};

class DroughtCreator : public WeatherCreator {
public:
    Weather* create() const override { return Drought::create(); }
};

class NormalWeatherCreator : public WeatherCreator {
public:
    Weather* create() const override { return NormalWeather::create(); }
};

/****************************************************************
 *
 * 使用工厂方法模式重构 - 重构后代码
 *
 ****************************************************************/
Weather* WeatherFactory::create(const std::string& type) {
    static RainCreator rainCreator;
    static DroughtCreator droughtCreator;
    static NormalWeatherCreator normalCreator;
    static const std::unordered_map<std::string, WeatherCreator*> creators = {
        { "rain", &rainCreator },
        { "drought", &droughtCreator },
        { "normal", &normalCreator }
    };

    auto it = creators.find(type);
    if (it != creators.end() && it->second) {
        return it->second->create();
    }
    return normalCreator.create();
}
