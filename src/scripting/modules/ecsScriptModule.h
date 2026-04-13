#pragma once
#include "scripting/scriptModule.h"

namespace ion
{
class EcsScriptModule final : public ScriptModule
{
public:
    void registerInternalCalls(ScriptAssembly &assembly) override;
};
}
