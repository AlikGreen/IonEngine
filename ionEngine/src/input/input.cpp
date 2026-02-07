#include "input.h"

#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    std::unordered_map<KeyCode, KeyState> Input::keyStates{};
    std::unordered_map<MouseButton, KeyState> Input::mouseButtonStates{};
    glm::vec2 Input::mousePosition{};
    glm::vec2 Input::mouseDelta{};

    bool Input::isKeyHeld(const KeyCode key)
    {
        return keyStates[key].held;
    }

    bool Input::isKeyReleased(const KeyCode key)
    {
        return keyStates[key].released;
    }

    bool Input::isKeyPressed(const KeyCode key)
    {
        return keyStates[key].pressed;
    }

    bool Input::isButtonHeld(const MouseButton btn)
    {
        return mouseButtonStates[btn].held;
    }

    bool Input::isButtonReleased(const MouseButton btn)
    {
        return mouseButtonStates[btn].released;
    }

    bool Input::isButtonPressed(const MouseButton btn)
    {
        return mouseButtonStates[btn].pressed;
    }

    glm::vec2 Input::getMousePosition()
    {
        return mousePosition;
    }

    glm::vec2 Input::getMouseDelta()
    {
        return mouseDelta;
    }

    void Input::setCursorLocked(const bool locked)
    {
        Engine::getSystem<GraphicsSystem>()->getWindow()->setCursorLocked(locked);
    }

    void Input::setCursorVisible(const bool visible)
    {
        Engine::getSystem<GraphicsSystem>()->getWindow()->setCursorVisible(visible);
    }
}
