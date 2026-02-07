#pragma once
#include <glm/glm.hpp>

#include "input/keyCodes.h"

namespace ion
{
    struct KeyState
    {
        bool pressed;
        bool released;
        bool held;
    };

class Input
{
public:
    static bool isKeyHeld(KeyCode key);
    static bool isKeyReleased(KeyCode key);
    static bool isKeyPressed(KeyCode key);

    static bool isButtonHeld(MouseButton btn);
    static bool isButtonReleased(MouseButton btn);
    static bool isButtonPressed(MouseButton btn);

    static glm::vec2 getMousePosition();
    static glm::vec2 getMouseDelta();

    static void setCursorLocked(bool locked);
    static void setCursorVisible(bool visible);
private:
    friend class InputSystem;
    static std::unordered_map<KeyCode, KeyState> keyStates;
    static std::unordered_map<MouseButton, KeyState> mouseButtonStates;
    static glm::vec2 mousePosition;
    static glm::vec2 mouseDelta;
};
}
