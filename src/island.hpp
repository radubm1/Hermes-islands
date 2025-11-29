#pragma once
#include "event_bus.hpp"
#include <string>
#include <cstddef>

class Island {
public:
    Island(std::string name, std::size_t heapBudgetBytes, EventBus& bus);

    bool loadModule(const std::string& jarPath);
    bool linkAll();
    bool unload();
    bool runMain(const std::string& mainClass);

private:
    std::string name_;
    std::size_t budget_;
    EventBus& bus_;
    bool loaded_ = false;
    bool linked_ = false;
};