// src/main.cpp
#include "vm.hpp"
#include <iostream>

EventBus* g_bus = nullptr; // definit global pentru bridge

int main() {
    VM vm;
    g_vm = &vm; 

    vm.events().on(EventType::INVOKE_START, [](const Event& e) {
        std::cout << "[Hermes] INVOKE_START: " << e.subject << "\n";
        });
    vm.events().on(EventType::RESULT, [](const Event& e) {
        std::cout << "[Hermes] RESULT: " << e.subject << "\n";
        });
    vm.events().on(EventType::INVOKE_END, [](const Event& e) {
        std::cout << "[Hermes] INVOKE_END: " << e.subject << "\n";
        });

    auto& A = vm.createIsland("A", 64 * 1024 * 1024);
    A.loadModule("plugins/meanrev-1.2.0.jar");
    A.linkAll();
    vm.runMain(A, "com/example/meanrev/Main");
    vm.unloadIsland(A);

    return 0;
}