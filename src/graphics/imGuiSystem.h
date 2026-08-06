#pragma once
#include <chrono>

#include "graphicsSystem.h"
#include "core/system.h"
#include "ImGuiImplDiligent.hpp"

#include "imgui.h"
#include "glm/glm.hpp"

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

    grl::Box<dg::ImGuiImplDiligent> m_imGuiController{};
    
    dg::Ref<dg::IRenderDevice> m_device{};
    dg::Ref<dg::ISwapChain> m_swapChain{};
    grl::Rc<Window> m_window{};

    dg::Ref<dg::ITexture> m_imguiTexture{};
    dg::Ref<dg::ITextureView> m_colorTextureView{};
    dg::Ref<dg::ISampler> m_colorTextureSampler{};

    GraphicsSystem* m_graphicsSystem{};

    std::string input{};
    glm::ivec3 inputSize{};

    size_t m_fps = 0;
    float m_frameTime = 0;
    size_t m_frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_frameCountStart{};
};
}
