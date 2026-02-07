#pragma once

namespace ion
{
class WindowResizeEvent final : public Event
{
public:
    WindowResizeEvent(const int width, const int height)
        : width(width), height(height) { }
    int width, height;
};
}
