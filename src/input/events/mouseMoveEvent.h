#pragma once
#include "core/event.h"

namespace ion
{
class MouseMoveEvent final : public Event
{
public:
    MouseMoveEvent(const float x, const float y) : x(x), y(y) { }

    [[nodiscard]] float getX() const { return x; }
    [[nodiscard]] float getY() const { return y; }
private:
    float x, y;
};
}
