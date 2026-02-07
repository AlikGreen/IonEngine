#pragma once
#include "input/keyCodes.h"


namespace ion
{
class KeyDownEvent final : public Event
{
public:
    KeyDownEvent(const KeyCode keycode, const bool repeat) : keycode(keycode), repeat(repeat) { };

    [[nodiscard]] KeyCode getKeycode() const { return keycode; }
    [[nodiscard]] bool isRepeat() const { return repeat; }
private:
    KeyCode keycode;
    bool repeat;
};
}
