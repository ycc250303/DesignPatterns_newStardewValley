#pragma once
#include <string>
class NPC;

class NPCCreator {
public:
    virtual ~NPCCreator() = default;
    virtual NPC* create() const = 0;
};

class NPCFactory {
public:
    static NPC* create(const std::string& id);
};
