#pragma once
#include "event_bus.hpp"
#include "island.hpp"
#include <memory>
#include <vector>

class VM {
public:
    Island& createIsland(const std::string& name, std::size_t budget);
    bool runMain(Island& isl, const std::string& mainClass);
    bool unloadIsland(Island& isl);

    EventBus& events() { return bus_; }

private:
    EventBus bus_;
    std::vector<std::unique_ptr<Island>> islands_;
};

extern VM* g_vm;
