#pragma once
#include "core/system.h"


namespace ion
{
class ScriptingSystem final : public System
{
public:
    void startup() override;
    void update() override;
    void shutdown() override;
};
}
