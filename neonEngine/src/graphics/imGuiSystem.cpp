#include "imGuiSystem.h"

#include "graphicsSystem.h"
#include "renderSystem.h"
#include "components/camera.h"
#include "core/sceneManager.h"
#include "events/rhiWindowEvent.h"
#include "imgui/imGuiExtensions.h"

namespace Neon
{
    ImFont* ImGuiSystem::headingFont{};
    ImFont* ImGuiSystem::subheadingFont{};
    ImFont* ImGuiSystem::regularFont{};
    ImFont* ImGuiSystem::smallFont{};

    void setNeonImGuiStyle()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        NeonGui::LoadStyle(AssetManager::getFullPath("style.yaml"), style);
    }

    ImGuiSystem::ImGuiSystem()
        : Stream(Level::Trace)
    {
    }

   void ImGuiSystem::preStartup()
    {
        m_graphicsSystem = Engine::getSystem<GraphicsSystem>();
        m_device = m_graphicsSystem->getDevice();
        m_window = m_graphicsSystem->getWindow();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.Fonts->Clear();

        static constexpr ImWchar rangesAllBMP[] =
        {
            0x0020, 0xFFFF,
            0
        };

        ImFontConfig baseCfg{};
        baseCfg.PixelSnapH = true;

        ImFontConfig mergeCfg{};
        mergeCfg.MergeMode = true;
        mergeCfg.PixelSnapH = true;

        auto addFontChecked = [&](const char* relPath, float size, ImFontConfig* cfg) -> ImFont*
        {
            const std::string fullPath = AssetManager::getFullPath(relPath);

            std::error_code ec{};
            if (!std::filesystem::exists(fullPath, ec) || std::filesystem::file_size(fullPath, ec) <= 100)
            {
                Log::error(std::string("Invalid font file: ") + fullPath);
                return nullptr;
            }

            return io.Fonts->AddFontFromFileTTF(fullPath.c_str(), size, cfg, rangesAllBMP);
        };

        auto addFontWithSymbols = [&](const char* baseFontPath, float size) -> ImFont*
        {
            ImFont* font = addFontChecked(baseFontPath, size, &baseCfg);
            if (font == nullptr)
                return nullptr;

            addFontChecked(R"(fonts\NotoSansSymbols2-Regular.ttf)", size, &mergeCfg);
            return font;
        };

        headingFont = addFontWithSymbols(R"(fonts\SpaceGrotesk-SemiBold.ttf)", 26.0f);
        subheadingFont = addFontWithSymbols(R"(fonts\SpaceGrotesk-Medium.ttf)", 24.0f);
        regularFont = addFontWithSymbols(R"(fonts\SpaceGrotesk-Regular.ttf)", 22.0f);
        smallFont = addFontWithSymbols(R"(fonts\JetBrainsMono-Regular.ttf)", 18.0f);

        io.FontDefault = regularFont;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        RHI::SamplerDesc samplerDesc{};
        samplerDesc.wrapMode.x = RHI::TextureWrap::ClampToEdge;
        samplerDesc.wrapMode.y = RHI::TextureWrap::ClampToEdge;
        m_colorTextureSampler = m_device->createSampler(samplerDesc);

        RHI::ImGuiController::InitInfo initInfo{};
        initInfo.device = m_device;
        initInfo.window = m_window;

        m_imGuiController = makeBox<RHI::ImGuiController>(initInfo);

        m_frameCountStart = std::chrono::high_resolution_clock::now();

        setNeonImGuiStyle();
    }

    void ImGuiSystem::update()
    {
        m_frameCount++;

        const std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();

        if(end - m_frameCountStart >= std::chrono::seconds(1))
        {
            m_fps = m_frameCount;
            m_frameTime = static_cast<float>((1.0 / static_cast<double>(m_fps)) * 1000.0);
            m_frameCountStart = end;
            m_frameCount = 0;
        }
    }

    void ImGuiSystem::render()
    {
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
            static_cast<float>(m_window->getWidth()),
            static_cast<float>(m_window->getHeight())
        );
        io.DeltaTime = Engine::getDeltaTime();

        m_imGuiController->newFrame();

        if(shouldDrawDockSpace)
            drawDockSpace();

        if(shouldDrawConsole)
            drawConsole();

        for(const auto& callback : renderCallbacks)
        {
            callback();
        }

        m_imGuiController->endFrame();

        const Rc<RHI::Texture> imGuiTexture = m_imGuiController->getFramebufferTexture();;
        if(!m_imguiTexture || m_imguiTexture != imGuiTexture)
        {
            m_imguiTexture = imGuiTexture;
            const auto viewDesc = RHI::TextureViewDesc(imGuiTexture);
            m_colorTextureView = m_device->createTextureView(viewDesc);
        }

        m_graphicsSystem->drawTexture(m_colorTextureView, m_colorTextureSampler);
    }

    void ImGuiSystem::addRenderCallback(const std::function<void()> &callback)
    {
        renderCallbacks.push_back(callback);
    }


    void ImGuiSystem::handle(std::string formattedMsg, std::string rawMsg, Level level)
    {
        consoleMessages.emplace_back(level, rawMsg);
    }

    void ImGuiSystem::drawDockSpace()
    {
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGuiDockNodeFlags dockspaceFlags = 0;
        dockspaceFlags |= ImGuiDockNodeFlags_PassthruCentralNode;

        const ImGuiViewport *viewport = ImGui::GetMainViewport();

        // Make host window + dockspace background transparent
        ImGui::PushStyleColor(ImGuiCol_WindowBg,      ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg,ImVec4(0, 0, 0, 0));

        // dockspace_id = 0 → ImGui will generate one
        ImGui::DockSpaceOverViewport(
            0,
            viewport,
            dockspaceFlags
        );

        ImGui::PopStyleColor(2);

    }

    void ImGuiSystem::drawConsole()
    {
        ImGui::Begin("Console");
        std::optional<size_t> pendingDeleteIndex;

        for (size_t i = 0; i < consoleMessages.size(); ++i)
        {
            const auto& [level, msg] = consoleMessages[i];

            ImGui::PushID(static_cast<int>(i));

            ImGui::Selectable(msg.c_str());

            if (ImGui::BeginPopupContextItem("MessageContext"))
            {
                if (ImGui::MenuItem("Copy"))
                {
                    ImGui::SetClipboardText(msg.c_str());
                }

                if (ImGui::MenuItem("Delete"))
                {
                    pendingDeleteIndex = i;
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        if (pendingDeleteIndex.has_value())
        {
            consoleMessages.erase(consoleMessages.begin() + static_cast<std::ptrdiff_t>(*pendingDeleteIndex));
        }
        ImGui::End();
    }

    void ImGuiSystem::event(Event *event)
    {
        if(const auto* rhiWindowEvent = dynamic_cast<RhiWindowEvent*>(event))
        {
            RHI::ImGuiController::processEvent(rhiWindowEvent->event);
        }
    }
}
