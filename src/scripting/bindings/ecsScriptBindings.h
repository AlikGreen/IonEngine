#pragma once
#include "scripting/ScriptBindings.h"

namespace ion
{
class EcsScriptBindings final : public ScriptBindings
{
public:
    void registerCalls(CallBinder &binder) override;
};
}
