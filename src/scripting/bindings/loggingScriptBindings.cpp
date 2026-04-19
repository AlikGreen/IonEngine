#include "loggingScriptBindings.h"

namespace ion
{
    void logInfo(const Coral::String message)
    {
        clogr::info("[C#] {}", static_cast<std::string>(message));
    }


    void LoggingScriptBindings::registerCalls(CallBinder &binder)
    {
        binder.bind<&logInfo>("IonEngine.Log", "logInfoCall");
    }
}
