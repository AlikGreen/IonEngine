#pragma once
#include "core/system.h"

namespace ion
{
class InputSystem final : public System
{
public:
    void event(Event *event) override;
    void postUpdate() override;
};
}
