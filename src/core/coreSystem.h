#pragma once
#include "system.h"

namespace ion
{
class CoreSystem final : public System
{
public:
    void event(Event* event) override;
};
}
