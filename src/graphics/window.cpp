#include "window.h"

#include "core/engine.h"
#include "core/eventManager.h"
#include "events/dropFileEvent.h"
#include "events/quitEvent.h"
#include "events/windowResizeEvent.h"
#include "input/events/keyDownEvent.h"
#include "input/events/keyUpEvent.h"
#include "input/events/mouseButtonDownEvent.h"
#include "input/events/mouseButtonUpEvent.h"
#include "input/events/mouseMoveEvent.h"
#include "input/events/mouseWheelEvent.h"
#include "input/events/textInputEvent.h"


namespace ion
{
    KeyCode keyCode(int glfwKey);
    MouseButton mouseButton(int glfwButton);
    KeyMod keyMod(int glfwMods);

    Window::Window(const WindowDesc &options)
    {
        const int suc = glfwInit();
        clogr::ensure(suc != 0, "Failed to initialize GLFW");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, options.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        m_width = options.width;
        m_height = options.height;

        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        clogr::ensure(primaryMonitor != nullptr, "Failed to get primary monitor - returned null");

        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
        clogr::ensure(mode != nullptr, "Failed get video mode - glfwGetVideoMode returned null");

        if(m_width <= 0)
            m_width = static_cast<int>(mode->width * 0.75);

        if(m_height <= 0)
            m_height = static_cast<int>(mode->height * 0.75);


        GLFWmonitor* monitor = options.fullscreen ? primaryMonitor : nullptr;
        m_handle = glfwCreateWindow(m_width, m_height, options.title, monitor, nullptr);
        clogr::ensure(m_handle != nullptr, "Failed to create GLFW window - glfwCreateWindow returned null");

        glfwSetWindowUserPointer(m_handle, this);

        glfwSetKeyCallback(        m_handle, keyCallback);
        glfwSetMouseButtonCallback(m_handle, mouseButtonCallback);
        glfwSetScrollCallback(     m_handle, scrollCallback);
        glfwSetCursorPosCallback(  m_handle, cursorPosCallback);
        glfwSetWindowSizeCallback( m_handle, windowSizeCallback);
        glfwSetWindowCloseCallback(m_handle, windowCloseCallback);
        glfwSetCharCallback(       m_handle, charCallback);
        glfwSetDropCallback(       m_handle, dropCallback);
    }

    void Window:: show()
    {
        glfwShowWindow(m_handle);
    }

    void Window::hide()
    {
        glfwHideWindow(m_handle);
    }

    void Window::close()
    {
        if(m_handle)
        {
            glfwDestroyWindow(m_handle);
            m_handle = nullptr;
        }
    }

    void Window::pollEvents()
    {
        glfwPollEvents();
    }

    int32_t Window::width()
    {
        return m_width;
    }

    int32_t Window::height()
    {
        return m_height;
    }

    void Window::width(const int32_t width)
    {
        glfwSetWindowSize(m_handle, width, m_height);
        m_width = width;
    }

    void Window::height(const int32_t height)
    {
        glfwSetWindowSize(m_handle, m_width, height);
        m_height = height;
    }

    std::string Window::title()
    {
        return glfwGetWindowTitle(m_handle);
    }

    void Window::title(const std::string& title)
    {
        glfwSetWindowTitle(m_handle, title.c_str());
    }

    void Window::setCursorLocked(const bool locked)
    {
        m_locked = locked;
        updateCursorState();
    }

    void Window::setCursorVisible(bool visible)
    {
        m_visible = visible;
        updateCursorState();
    }


    void Window::updateCursorState() const
    {
        if(m_locked && m_visible)
            glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
        if(m_locked && !m_visible)
            glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if(!m_locked && m_visible)
            glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        if(!m_locked && !m_visible)
            glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    void Window::keyCallback(GLFWwindow* windowGLFW, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            Engine::eventManager().queueEvent<KeyDownEvent>(keyCode(key), action == GLFW_REPEAT);
        else if (action == GLFW_RELEASE)
            Engine::eventManager().queueEvent<KeyUpEvent>(keyCode(key));
    }

    void Window::mouseButtonCallback(GLFWwindow* windowGLFW, int button, int action, int mods)
    {
        if (action == GLFW_PRESS)
            Engine::eventManager().queueEvent<MouseButtonDownEvent>(mouseButton(button));
        else if (action == GLFW_RELEASE)
            Engine::eventManager().queueEvent<MouseButtonUpEvent>(mouseButton(button));
    }

    void Window::cursorPosCallback(GLFWwindow* windowGLFW, const double xPos, const double yPos)
    {
        Engine::eventManager().queueEvent<MouseMoveEvent>(static_cast<float>(xPos), static_cast<float>(yPos));
    }

    void Window::windowSizeCallback(GLFWwindow* windowGLFW, const int width, const int height)
    {
        Engine::eventManager().queueEvent<WindowResizeEvent>(width, height);
    }

    void Window::windowCloseCallback(GLFWwindow* windowGLFW)
    {
        Engine::eventManager().queueEvent<QuitEvent>();
    }

    void Window::charCallback(GLFWwindow* windowGLFW, const unsigned int codepoint)
    {
        Engine::eventManager().queueEvent<TextInputEvent>(codepoint);
    }

    void Window::scrollCallback(GLFWwindow* windowGLFW, const double xOffset, const double yOffset)
    {
        Engine::eventManager().queueEvent<MouseWheelEvent>(xOffset, yOffset);
    }

    void Window::dropCallback(GLFWwindow* windowGLFW, int pathCount, const char** paths)
    {
        for (int i = 0; i < pathCount; ++i)
        {
            const char* path = paths[i];
            Engine::eventManager().queueEvent<DropFileEvent>(path);
        }
    }

    KeyCode keyCode(const int glfwKey)
    {
        switch (glfwKey)
        {
            case GLFW_KEY_ENTER:       return KeyCode::Return;
            case GLFW_KEY_ESCAPE:      return KeyCode::Escape;
            case GLFW_KEY_BACKSPACE:   return KeyCode::Backspace;
            case GLFW_KEY_TAB:         return KeyCode::Tab;
            case GLFW_KEY_SPACE:       return KeyCode::Space;
            case GLFW_KEY_APOSTROPHE:  return KeyCode::Apostrophe;
            case GLFW_KEY_COMMA:       return KeyCode::Comma;
            case GLFW_KEY_MINUS:       return KeyCode::Minus;
            case GLFW_KEY_PERIOD:      return KeyCode::Period;
            case GLFW_KEY_SLASH:       return KeyCode::Slash;
            case GLFW_KEY_0:           return KeyCode::Num0;
            case GLFW_KEY_1:           return KeyCode::Num1;
            case GLFW_KEY_2:           return KeyCode::Num2;
            case GLFW_KEY_3:           return KeyCode::Num3;
            case GLFW_KEY_4:           return KeyCode::Num4;
            case GLFW_KEY_5:           return KeyCode::Num5;
            case GLFW_KEY_6:           return KeyCode::Num6;
            case GLFW_KEY_7:           return KeyCode::Num7;
            case GLFW_KEY_8:           return KeyCode::Num8;
            case GLFW_KEY_9:           return KeyCode::Num9;
            case GLFW_KEY_SEMICOLON:   return KeyCode::Semicolon;
            case GLFW_KEY_EQUAL:       return KeyCode::Equals;
            case GLFW_KEY_LEFT_BRACKET:return KeyCode::LeftBracket;
            case GLFW_KEY_BACKSLASH:   return KeyCode::Backslash;
            case GLFW_KEY_RIGHT_BRACKET:return KeyCode::RightBracket;
            case GLFW_KEY_GRAVE_ACCENT:return KeyCode::Grave;
            case GLFW_KEY_A:           return KeyCode::A;
            case GLFW_KEY_B:           return KeyCode::B;
            case GLFW_KEY_C:           return KeyCode::C;
            case GLFW_KEY_D:           return KeyCode::D;
            case GLFW_KEY_E:           return KeyCode::E;
            case GLFW_KEY_F:           return KeyCode::F;
            case GLFW_KEY_G:           return KeyCode::G;
            case GLFW_KEY_H:           return KeyCode::H;
            case GLFW_KEY_I:           return KeyCode::I;
            case GLFW_KEY_J:           return KeyCode::J;
            case GLFW_KEY_K:           return KeyCode::K;
            case GLFW_KEY_L:           return KeyCode::L;
            case GLFW_KEY_M:           return KeyCode::M;
            case GLFW_KEY_N:           return KeyCode::N;
            case GLFW_KEY_O:           return KeyCode::O;
            case GLFW_KEY_P:           return KeyCode::P;
            case GLFW_KEY_Q:           return KeyCode::Q;
            case GLFW_KEY_R:           return KeyCode::R;
            case GLFW_KEY_S:           return KeyCode::S;
            case GLFW_KEY_T:           return KeyCode::T;
            case GLFW_KEY_U:           return KeyCode::U;
            case GLFW_KEY_V:           return KeyCode::V;
            case GLFW_KEY_W:           return KeyCode::W;
            case GLFW_KEY_X:           return KeyCode::X;
            case GLFW_KEY_Y:           return KeyCode::Y;
            case GLFW_KEY_Z:           return KeyCode::Z;
            case GLFW_KEY_DELETE:      return KeyCode::Delete;

            // Function keys
            case GLFW_KEY_CAPS_LOCK:   return KeyCode::CapsLock;
            case GLFW_KEY_F1:          return KeyCode::F1;
            case GLFW_KEY_F2:          return KeyCode::F2;
            case GLFW_KEY_F3:          return KeyCode::F3;
            case GLFW_KEY_F4:          return KeyCode::F4;
            case GLFW_KEY_F5:          return KeyCode::F5;
            case GLFW_KEY_F6:          return KeyCode::F6;
            case GLFW_KEY_F7:          return KeyCode::F7;
            case GLFW_KEY_F8:          return KeyCode::F8;
            case GLFW_KEY_F9:          return KeyCode::F9;
            case GLFW_KEY_F10:         return KeyCode::F10;
            case GLFW_KEY_F11:         return KeyCode::F11;
            case GLFW_KEY_F12:         return KeyCode::F12;
            case GLFW_KEY_F13:         return KeyCode::F13;
            case GLFW_KEY_F14:         return KeyCode::F14;
            case GLFW_KEY_F15:         return KeyCode::F15;
            case GLFW_KEY_F16:         return KeyCode::F16;
            case GLFW_KEY_F17:         return KeyCode::F17;
            case GLFW_KEY_F18:         return KeyCode::F18;
            case GLFW_KEY_F19:         return KeyCode::F19;
            case GLFW_KEY_F20:         return KeyCode::F20;
            case GLFW_KEY_F21:         return KeyCode::F21;
            case GLFW_KEY_F22:         return KeyCode::F22;
            case GLFW_KEY_F23:         return KeyCode::F23;
            case GLFW_KEY_F24:         return KeyCode::F24;
            case GLFW_KEY_F25:         return KeyCode::F25;

            case GLFW_KEY_PRINT_SCREEN:return KeyCode::PrintScreen;
            case GLFW_KEY_SCROLL_LOCK: return KeyCode::ScrollLock;
            case GLFW_KEY_PAUSE:       return KeyCode::Pause;
            case GLFW_KEY_INSERT:      return KeyCode::Insert;
            case GLFW_KEY_HOME:        return KeyCode::Home;
            case GLFW_KEY_PAGE_UP:     return KeyCode::PageUp;
            case GLFW_KEY_END:         return KeyCode::End;
            case GLFW_KEY_PAGE_DOWN:   return KeyCode::PageDown;
            case GLFW_KEY_RIGHT:       return KeyCode::Right;
            case GLFW_KEY_LEFT:        return KeyCode::Left;
            case GLFW_KEY_DOWN:        return KeyCode::Down;
            case GLFW_KEY_UP:          return KeyCode::Up;
            case GLFW_KEY_NUM_LOCK:    return KeyCode::NumLockClear;
            case GLFW_KEY_KP_DIVIDE:   return KeyCode::KpDivide;
            case GLFW_KEY_KP_MULTIPLY: return KeyCode::KpMultiply;
            case GLFW_KEY_KP_SUBTRACT: return KeyCode::KpMinus;
            case GLFW_KEY_KP_ADD:      return KeyCode::KpPlus;
            case GLFW_KEY_KP_ENTER:    return KeyCode::KpEnter;
            case GLFW_KEY_KP_1:        return KeyCode::Kp1;
            case GLFW_KEY_KP_2:        return KeyCode::Kp2;
            case GLFW_KEY_KP_3:        return KeyCode::Kp3;
            case GLFW_KEY_KP_4:        return KeyCode::Kp4;
            case GLFW_KEY_KP_5:        return KeyCode::Kp5;
            case GLFW_KEY_KP_6:        return KeyCode::Kp6;
            case GLFW_KEY_KP_7:        return KeyCode::Kp7;
            case GLFW_KEY_KP_8:        return KeyCode::Kp8;
            case GLFW_KEY_KP_9:        return KeyCode::Kp9;
            case GLFW_KEY_KP_0:        return KeyCode::Kp0;
            case GLFW_KEY_KP_DECIMAL:  return KeyCode::KpPeriod;
            case GLFW_KEY_KP_EQUAL:    return KeyCode::KpEquals;
            case GLFW_KEY_MENU:        return KeyCode::Menu;

            // Modifier keys
            case GLFW_KEY_LEFT_CONTROL: return KeyCode::LCtrl;
            case GLFW_KEY_LEFT_SHIFT:   return KeyCode::LShift;
            case GLFW_KEY_LEFT_ALT:     return KeyCode::LAlt;
            case GLFW_KEY_LEFT_SUPER:   return KeyCode::LGui;
            case GLFW_KEY_RIGHT_CONTROL:return KeyCode::RCtrl;
            case GLFW_KEY_RIGHT_SHIFT:  return KeyCode::RShift;
            case GLFW_KEY_RIGHT_ALT:    return KeyCode::RAlt;
            case GLFW_KEY_RIGHT_SUPER:  return KeyCode::RGui;

            // Note: GLFW doesn't have direct equivalents for many of the specialized SDL keys
            // like media keys, special symbols, or advanced keypad functions
            // These would need to be handled through character input callbacks if needed

            default:
                return KeyCode::Unknown;
        }
    }

    KeyMod keyMod(const int glfwMods)
    {
        auto mod = KeyMod::None;
        if (glfwMods & GLFW_MOD_SHIFT)     mod = static_cast<KeyMod>(static_cast<uint16_t>(mod) | static_cast<uint16_t>(KeyMod::LShift));
        if (glfwMods & GLFW_MOD_CONTROL)   mod = static_cast<KeyMod>(static_cast<uint16_t>(mod) | static_cast<uint16_t>(KeyMod::LCtrl));
        if (glfwMods & GLFW_MOD_ALT)       mod = static_cast<KeyMod>(static_cast<uint16_t>(mod) | static_cast<uint16_t>(KeyMod::LAlt));
        if (glfwMods & GLFW_MOD_SUPER)     mod = static_cast<KeyMod>(static_cast<uint16_t>(mod) | static_cast<uint16_t>(KeyMod::LGui));
        if (glfwMods & GLFW_MOD_CAPS_LOCK) mod = static_cast<KeyMod>(static_cast<uint16_t>(mod) | static_cast<uint16_t>(KeyMod::Caps));
        if (glfwMods & GLFW_MOD_NUM_LOCK)  mod = static_cast<KeyMod>(static_cast<uint16_t>(mod) | static_cast<uint16_t>(KeyMod::Num));

        // Note: GLFW doesn't distinguish between left/right modifiers in the mods parameter
        // and doesn't have direct equivalents for Mode or Scroll lock modifiers

        return mod;
    }

    MouseButton mouseButton(const int glfwButton)
    {
        switch (glfwButton)
        {
            case GLFW_MOUSE_BUTTON_LEFT:   return MouseButton::Left;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
            case GLFW_MOUSE_BUTTON_RIGHT:  return MouseButton::Right;
            case GLFW_MOUSE_BUTTON_4:      return MouseButton::Side1;
            case GLFW_MOUSE_BUTTON_5:      return MouseButton::Side2;
            // GLFW supports buttons 6-8 as well if your MouseButton enum has more
            default:                       return MouseButton::None;
        }
    }
}
