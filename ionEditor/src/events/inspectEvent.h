#pragma once
#include <any>

#include "core/event.h"

namespace ion
{
class InspectEvent final : public Event
{
public:
    explicit InspectEvent(const std::variant<entis::Entity, AssetId> &inspected)
        : inspected(inspected)
    {  }

    std::variant<entis::Entity, AssetId> inspected;
};
}
