#pragma once
#include <chrono>

#include "graphicsSystem.h"
#include "imgui/imGuiController.h"
#include "core/system.h"
#include "imgui/imGuiConfig.h"
#include <clogr.h>

namespace ion
{
class ImGuiSystem final : public System
{
public:
    struct ConsoleMessage
    {
        std::string unformatted;
        std::string formatted;
        clogr::Level level;
    };

    void preStartup() override;
    void update() override;
    void render() override;

    void addRenderCallback(const std::function<void()> &callback);
    void onMessage(const ConsoleMessage &message);

    void event(Event *event) override;

    bool shouldDrawDockSpace = false;
    bool shouldDrawConsole = false;
    bool shouldDrawStats = false;

    static ImFont* headingFont;
    static ImFont* subheadingFont;
    static ImFont* regularFont;
    static ImFont* smallFont;
private:
    static void drawDockSpace();
    void drawConsole();

    static std::pair<std::string, std::string> splitStringByMaxWidth(const std::string &input, float maxWidth);

    std::vector<ConsoleMessage> consoleMessages{};
    std::vector<std::function<void()>> renderCallbacks{};

    grl::Box<urhi::ImGuiController> m_imGuiController{};
    grl::Rc<urhi::Device> m_device{};
    grl::Rc<urhi::Window> m_window{};

    grl::Rc<urhi::Texture> m_imguiTexture{};
    grl::Rc<urhi::TextureView> m_colorTextureView{};
    grl::Rc<urhi::Sampler> m_colorTextureSampler{};

    GraphicsSystem* m_graphicsSystem{};

    std::string input{};
    glm::ivec3 inputSize{};

    size_t m_fps = 0;
    float m_frameTime = 0;
    size_t m_frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_frameCountStart{};
};
}
