#include "eventManager.h"

#include <ranges>

#include "core/engine.h"

namespace ion
{
    void EventManager::handleEvents()
    {
        for(const auto& event : events)
        {
            for (const auto& system : std::ranges::reverse_view(Engine::getSystems()))
            {
                system->event(event.get());

                if(event->isCanceled())
                    break;
            }
        }

        events.clear();
    }
}
