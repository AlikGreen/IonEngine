#pragma once
#include "scriptAssembly.h"

namespace ion
{
class ScriptModule
{
public:
  virtual ~ScriptModule() = default;

  virtual void registerInternalCalls(ScriptAssembly& assembly) = 0;
  virtual void onAssemblyLoaded(ScriptAssembly& assembly) { };
  virtual void onAssemblyUnloaded() { }
};
}
