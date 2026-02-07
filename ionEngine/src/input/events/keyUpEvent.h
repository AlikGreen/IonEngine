#pragma once

#include "core/event.h"

namespace ion
{
    class KeyUpEvent final : public Event
    {
    public:
        explicit KeyUpEvent(const KeyCode keycode) : keycode(keycode) { };

        [[nodiscard]] KeyCode getKeycode() const { return keycode; }
    private:
        KeyCode keycode;
    };
}
