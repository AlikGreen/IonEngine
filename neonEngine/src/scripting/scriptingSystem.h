#pragma once
#include <functional>

#include "core/system.h"
#include <windows.h>

#include "log.h"

namespace Neon
{
class ScriptingSystem final : public System
{
public:
    void startup() override;
    void update() override;
    void shutdown() override;
};
}
