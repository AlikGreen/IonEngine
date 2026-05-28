#pragma once
#include "scripting/scriptBindings.h"

namespace ion
{
class MeshScriptBindings final : public ScriptBindings
{
public:
    void registerCalls(CallBinder &binder) override;
};
}
