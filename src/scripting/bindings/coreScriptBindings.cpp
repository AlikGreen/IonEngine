#include "coreScriptBindings.h"

#include "core/engine.h"

namespace ion
{
    float Engine_getDeltaTime()
    {
        return Engine::getDeltaTime();
    }

    void CoreScriptBindings::registerCalls(CallBinder &binder)
    {
        binder.bind<&Engine_getDeltaTime>("IonEngine.NativeBridge", "Engine_getDeltaTime");
    }
}
