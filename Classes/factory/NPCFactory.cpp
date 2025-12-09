#include "factory/NPCFactory.h"
#include "Alex.h"
#include "Lewis.h"
#include "Marlon.h"
#include "Maru.h"
#include "Pig.h"
#include "Chicken.h"
#include "Sheep.h"
#include <unordered_map>
#include <string>

class LewisCreator : public NPCCreator { public: NPC* create() const override { return Lewis::create(); } };
class MarlonCreator : public NPCCreator { public: NPC* create() const override { return Marlon::create(); } };
class MaruCreator   : public NPCCreator { public: NPC* create() const override { return Maru::create(); } };
class AlexCreator   : public NPCCreator { public: NPC* create() const override { return Alex::create(); } };
class PigCreator    : public NPCCreator { public: NPC* create() const override { return Pig::create(); } };
class ChickenCreator: public NPCCreator { public: NPC* create() const override { return Chicken::create(); } };
class SheepCreator  : public NPCCreator { public: NPC* create() const override { return Sheep::create(); } };

/****************************************************************
 *
 * 使用工厂方法模式重构 - 重构后代码
 *
 ****************************************************************/
NPC* NPCFactory::create(const std::string& id) {
    static LewisCreator lewisCreator;
    static MarlonCreator marlonCreator;
    static MaruCreator maruCreator;
    static AlexCreator alexCreator;
    static PigCreator pigCreator;
    static ChickenCreator chickenCreator;
    static SheepCreator sheepCreator;

    static const std::unordered_map<std::string, NPCCreator*> creators = {
        { "lewis", &lewisCreator },
        { "marlon", &marlonCreator },
        { "maru", &maruCreator },
        { "alex", &alexCreator },
        { "pig", &pigCreator },
        { "chicken", &chickenCreator },
        { "sheep", &sheepCreator }
    };

    auto it = creators.find(id);
    if (it != creators.end() && it->second) {
        return it->second->create();
    }
    return nullptr;
}
