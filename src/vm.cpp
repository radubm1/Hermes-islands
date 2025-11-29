#include "vm.hpp"

VM* g_vm = nullptr;

Island& VM::createIsland(const std::string& name, std::size_t budget) {
    islands_.push_back(std::make_unique<Island>(name, budget, bus_));
    return *islands_.back();
}

bool VM::runMain(Island& isl, const std::string& mainClass) {
    return isl.runMain(mainClass);
}

bool VM::unloadIsland(Island& isl) {
    return isl.unload();
}