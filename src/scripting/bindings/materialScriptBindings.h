#pragma once
#include "scripting/scriptBindings.h"

namespace ion
{
class MaterialScriptBindings final : public ScriptBindings
{
public:
    void registerCalls(CallBinder &binder) override;
};
}
