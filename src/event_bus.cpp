#include "event_bus.hpp"

void EventBus::on(EventType t, Handler handler) {
    handlers_[t].push_back(std::move(handler));
}

void EventBus::emit(const Event& e) {
    auto it = handlers_.find(e.type);
    if (it == handlers_.end()) return;
    for (auto& h : it->second) {
        h(e);
    }
}