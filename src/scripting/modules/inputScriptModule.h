#pragma once
#include "scripting/scriptModule.h"

namespace ion
{
class InputScriptModule final : public ScriptModule
{
public:
    void registerInternalCalls(ScriptAssembly &assembly) override;
    void onAssemblyLoaded(ScriptAssembly &assembly) override;
};
}
