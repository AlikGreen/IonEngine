#pragma once
#include "scripting/scriptBindings.h"

namespace ion
{
class CoreScriptBindings final : public ScriptBindings
{
public:
    void registerCalls(CallBinder &binder) override;
};
}
