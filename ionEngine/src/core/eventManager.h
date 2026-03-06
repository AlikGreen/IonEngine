#pragma once
#include <vector>

#include "event.h"

namespace ion
{
class EventManager
{
public:
    EventManager() = default;

    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    template<typename T, typename... Args>
    requires std::is_base_of_v<Event, T> && std::is_constructible_v<T, Args...>
    void queueEvent(Args&&... args)
    {
        events.push_back(grl::makeBox<T>(std::forward<Args>(args)...));
    }

    void handleEvents();
private:
    std::vector<grl::Box<Event>> events { };
};
}
