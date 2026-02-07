#include "inputSystem.h"

#include <ranges>

#include "input.h"
#include "events/keyDownEvent.h"
#include "events/keyUpEvent.h"
#include "events/mouseButtonDownEvent.h"
#include "events/mouseButtonUpEvent.h"
#include "events/mouseMoveEvent.h"

namespace ion
{
    void InputSystem::event(Event *event)
    {
        if(const auto* keyDownEvent = dynamic_cast<KeyDownEvent*>(event))
        {
            if(keyDownEvent->isRepeat()) return;
            Input::keyStates[keyDownEvent->getKeycode()].held = true;
            Input::keyStates[keyDownEvent->getKeycode()].pressed = true;
        }else if(const auto* keyUpEvent = dynamic_cast<KeyUpEvent*>(event))
        {
            Input::keyStates[keyUpEvent->getKeycode()].held = false;
            Input::keyStates[keyUpEvent->getKeycode()].released = true;
        }
        else if(const auto* mouseDownEvent = dynamic_cast<MouseButtonDownEvent*>(event))
        {
            Input::mouseButtonStates[mouseDownEvent->getButton()].held = true;
            Input::mouseButtonStates[mouseDownEvent->getButton()].pressed = true;
        }else if(const auto* mouseUpEvent = dynamic_cast<MouseButtonUpEvent*>(event))
        {
            Input::mouseButtonStates[mouseUpEvent->getButton()].held = false;
            Input::mouseButtonStates[mouseUpEvent->getButton()].released = true;
        }else if(const auto* mouseMoveEvent = dynamic_cast<MouseMoveEvent*>(event))
        {
            const glm::vec2 newPos = {mouseMoveEvent->getX(), mouseMoveEvent->getY()};
            Input::mouseDelta += newPos-Input::mousePosition;
            Input::mousePosition = newPos;
        }
    }

    void InputSystem::postUpdate()
    {
        for (auto& keyState: std::views::values(Input::keyStates))
        {
            keyState.pressed = false;
            keyState.released = false;
        }

        for (auto& keyState: std::views::values(Input::mouseButtonStates))
        {
            keyState.pressed = false;
            keyState.released = false;
        }

        Input::mouseDelta = glm::vec2(0.0f);
    }
}
