#pragma once
#include "scripting/scriptBindings.h"

namespace ion
{
class InputScriptBindings final : public ScriptBindings
{
public:
    void registerCalls(CallBinder &binder) override;
};
}
