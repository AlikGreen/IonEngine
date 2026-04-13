#pragma once
#include "scripting/scriptModule.h"

namespace ion
{
class LoggingScriptModule final : public ScriptModule
{
public:
    void registerInternalCalls(ScriptAssembly &assembly) override;
};
}
