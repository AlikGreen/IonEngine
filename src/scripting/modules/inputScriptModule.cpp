#include "inputScriptModule.h"

#include "input/input.h"

namespace ion
{
    bool isKeyHeld(int key)
    {
        return Input::isKeyHeld(static_cast<KeyCode>(key));
    }

    bool isKeyPressed(int key)
    {
        return Input::isKeyPressed(static_cast<KeyCode>(key));
    }

    bool isKeyReleased(int key)
    {
        return Input::isKeyReleased(static_cast<KeyCode>(key));
    }

    void InputScriptModule::registerInternalCalls(ScriptAssembly &assembly)
    {
        assembly.addInternalCall<&isKeyHeld>("IonEngine.Input", "isKeyHeldCall");
        assembly.addInternalCall<&isKeyPressed>("IonEngine.Input", "isKeyPressedCall");
        assembly.addInternalCall<&isKeyReleased>("IonEngine.Input", "isKeyReleasedCall");
    }

    void InputScriptModule::onAssemblyLoaded(ScriptAssembly &assembly)
    {
        auto inputClass = assembly.getType("IonEngine.Input");
        inputClass.invokeStaticMethod("Run");
    }
}
