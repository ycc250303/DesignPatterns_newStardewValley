#pragma once
#include <string>
class Weather;

class WeatherCreator {
public:
    virtual ~WeatherCreator() = default;
    virtual Weather* create() const = 0;
};

class WeatherFactory {
public:
    static Weather* create(const std::string& type);
};
