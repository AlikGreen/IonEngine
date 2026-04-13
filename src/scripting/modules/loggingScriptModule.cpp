#include "loggingScriptModule.h"

namespace ion
{
    void logInfo(const Coral::String message)
    {
        clogr::info("[C#] {}", static_cast<std::string>(message));
    }

    void LoggingScriptModule::registerInternalCalls(ScriptAssembly &assembly)
    {
        assembly.addInternalCall<&logInfo>("IonEngine.Log", "logInfoCall");
        assembly.uploadInternalCalls();
    }
}
