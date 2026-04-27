#include "inputScriptBindings.h"

#include "input/input.h"
#include "input/keyCodes.h"

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

    void InputScriptBindings::registerCalls(CallBinder &binder)
    {
        binder.bind<&isKeyHeld>("IonEngine.NativeBridge", "Input_isKeyHeld");
        binder.bind<&isKeyPressed>("IonEngine.NativeBridge", "Input_isKeyPressed");
        binder.bind<&isKeyReleased>("IonEngine.NativeBridge", "Input_isKeyReleased");
    }
}
