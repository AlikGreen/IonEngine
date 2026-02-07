#pragma once

#include "core/event.h"


namespace ion
{
class TextInputEvent final : public Event
{
public:
    explicit TextInputEvent(const uint32_t codepoint) : codepoint(codepoint) { };

    [[nodiscard]] uint32_t getCodepoint() const { return codepoint; }
private:
    const uint32_t codepoint;
};
}
