#pragma once

#include "core/event.h"

namespace ion
{
    class MouseButtonUpEvent final : public Event
    {
    public:
        explicit MouseButtonUpEvent(const MouseButton keycode) : button(keycode) { };

        [[nodiscard]] MouseButton getButton() const { return button; }
    private:
        MouseButton button;
    };
}
