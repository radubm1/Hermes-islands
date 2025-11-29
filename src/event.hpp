#pragma once
#include <string>
#include <cstddef>

enum class EventType {
    LOAD_REQUEST,
    LOAD_OK,
    LOAD_FAIL,
    VERIFY_OK,
    LINK_OK,
    INVOKE_START,
    INVOKE_END,
    ALLOC,
    GC_MINOR,
    GC_MAJOR,
    UNLOAD_REQUEST,
    UNLOAD_OK,
    UNLOAD_FAIL,
    RESULT   // <- adaugă aici
};

struct Event {
    EventType type;
    std::string subject;
    std::size_t value = 0;

    Event() = default;
    Event(EventType t, const std::string& s, std::size_t v = 0)
        : type(t), subject(s), value(v) {}
};