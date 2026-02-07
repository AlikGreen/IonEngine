#pragma once
#include <neonRHI/neonRHI.h>
#include "core/event.h"

namespace ion
{
    class RhiWindowEvent final : public Event
    {
    public:
        explicit RhiWindowEvent(const urhi::Event &event) : event(event) {  }
        urhi::Event event;
    };
}
