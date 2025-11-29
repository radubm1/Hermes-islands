#pragma once
#include "event.hpp"
#include <functional>
#include <unordered_map>
#include <vector>

// hash pentru enum class EventType (cheie în unordered_map)
struct EventTypeHash {
    std::size_t operator()(EventType t) const noexcept {
        return static_cast<std::size_t>(t);
    }
};

class EventBus {
public:
    using Handler = std::function<void(const Event&)>;

    void on(EventType t, Handler handler);
    void emit(const Event& e);

private:
    std::unordered_map<EventType, std::vector<Handler>, EventTypeHash> handlers_;
};