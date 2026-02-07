#pragma once
#include "core/event.h"

namespace ion
{
class MouseWheelEvent final : public Event
{
public:
    MouseWheelEvent(const int x, const int y) : x(x), y(y) { }

    [[nodiscard]] int getX() const { return x; }
    [[nodiscard]] int getY() const { return y; }
private:
    int x, y;
};
}
