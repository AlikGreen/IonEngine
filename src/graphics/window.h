#pragma once

#include "GLFW/glfw3.h"

namespace ion
{
struct WindowDesc
{
    const char* title{};
    int width = -1;
    int height = -1;
    bool fullscreen = false;
    bool resizable = true;
};

class Window
{
public:
    explicit Window(const WindowDesc& options);

    void show();
    void hide();

    void close();

    void pollEvents();

    int32_t width();
    int32_t height();

    void width(int32_t width);
    void height(int32_t height);

    std::string title();
    void title(const std::string& title);

    void setCursorLocked(bool locked);
    void setCursorVisible(bool visible);

    [[nodiscard]] GLFWwindow* handle() const { return m_handle; }
private:
    void updateCursorState() const;

    GLFWwindow* m_handle = nullptr;

    int32_t m_width{};
    int32_t m_height{};

    bool m_locked = false;
    bool m_visible = true;

    static void keyCallback(GLFWwindow* windowGLFW, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* windowGLFW, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* windowGLFW, double xPos, double yPos);
    static void windowSizeCallback(GLFWwindow* windowGLFW, int width, int height);
    static void windowCloseCallback(GLFWwindow* windowGLFW);
    static void charCallback(GLFWwindow* windowGLFW, unsigned int codepoint);
    static void scrollCallback(GLFWwindow *windowGLFW, double xOffset, double yOffset);
    static void dropCallback(GLFWwindow *windowGLFW, int pathCount, const char **paths);
};
}
