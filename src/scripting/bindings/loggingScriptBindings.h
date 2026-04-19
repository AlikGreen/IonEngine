#pragma once
#include "scripting/scriptBindings.h"

namespace ion
{
class LoggingScriptBindings final : public ScriptBindings
{
public:
    void registerCalls(CallBinder &binder) override;
};
}
