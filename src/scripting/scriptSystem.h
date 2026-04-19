#pragma once
#include "core/system.h"

namespace ion
{
class ScriptSystem final : public System
{
public:
    void sceneLoaded(Scene &scene) override;
    void sceneUnloaded(Scene &scene) override;

    void update() override;
};
}
