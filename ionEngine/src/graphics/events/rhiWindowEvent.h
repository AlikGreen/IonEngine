#pragma once
#include <urhi/urhi.h>

#include <utility>
#include "core/event.h"

namespace ion
{
    class RhiWindowEvent final : public Event
    {
    public:
        explicit RhiWindowEvent(urhi::Event event) : event(std::move(event)) {  }
        urhi::Event event;
    };
}
