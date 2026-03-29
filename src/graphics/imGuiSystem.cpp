#include "imGuiSystem.h"

#include "consoleWindowSink.h"
#include "graphicsSystem.h"
#include "imGuiStyleSerializer.h"
#include "components/camera.h"
#include "core/sceneManager.h"
#include "events/rhiWindowEvent.h"
#include "imgui/imGuiExtensions.h"
#include "util/ansiParser.h"

namespace ion
{
    ImFont* ImGuiSystem::headingFont{};
    ImFont* ImGuiSystem::subheadingFont{};
    ImFont* ImGuiSystem::regularFont{};
    ImFont* ImGuiSystem::smallFont{};

    void setNeonImGuiStyle()
    {
        ImGuiStyle &style = ImGui::GetStyle();
        ImGuiStyleSerializer::deserialize(Engine::getResourceFS().resolve("style.json"), style);
    }

   void ImGuiSystem::preStartup()
    {
        m_graphicsSystem = Engine::getSystem<GraphicsSystem>();
        m_device = m_graphicsSystem->getDevice();
        m_window = m_graphicsSystem->getWindow();

        clogr::defaultLogger()->addSink<ConsoleWindowSink>();

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
            const std::string fullPath = Engine::getResourceFS().resolve(relPath).string();

            std::error_code ec{};
            if (!std::filesystem::exists(fullPath, ec) || std::filesystem::file_size(fullPath, ec) <= 100)
            {
                clogr::error(std::string("Invalid font file: ") + fullPath);
                return nullptr;
            }

            return io.Fonts->AddFontFromFileTTF(fullPath.c_str(), size, cfg, rangesAllBMP);
        };

        auto addFontWithSymbols = [&](const char* baseFontPath, const float size) -> ImFont*
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

        urhi::SamplerDesc samplerDesc{};
        samplerDesc.addressModeU = urhi::AddressMode::ClampToEdge;
        samplerDesc.addressModeV = urhi::AddressMode::ClampToEdge;
        m_colorTextureSampler = m_device->createSampler(samplerDesc);

        urhi::ImGuiController::InitInfo initInfo{};
        initInfo.device = m_device;
        initInfo.window = m_window;

        m_imGuiController = grl::makeBox<urhi::ImGuiController>(initInfo);

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
            static_cast<float>(m_window->width()),
            static_cast<float>(m_window->height())
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

        const grl::Rc<urhi::Texture> imGuiTexture = m_imGuiController->getFramebufferTexture();;
        if(!m_imguiTexture || m_imguiTexture != imGuiTexture)
        {
            m_imguiTexture = imGuiTexture;
            const auto viewDesc = urhi::TextureViewDesc(imGuiTexture);
            m_colorTextureView = m_device->createTextureView(viewDesc);
        }

        m_graphicsSystem->drawTexture(m_colorTextureView->texture());
    }

    void ImGuiSystem::addRenderCallback(const std::function<void()> &callback)
    {
        renderCallbacks.push_back(callback);
    }

    void ImGuiSystem::onMessage(const ConsoleMessage &message)
    {
        consoleMessages.push_back(message);
    }

    void ImGuiSystem::drawDockSpace()
    {
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGuiDockNodeFlags dockspaceFlags = 0;
        dockspaceFlags |= ImGuiDockNodeFlags_PassthruCentralNode;

        const ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::PushStyleColor(ImGuiCol_WindowBg,      ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg,ImVec4(0, 0, 0, 0));

        ImGui::DockSpaceOverViewport(
            0,
            viewport,
            dockspaceFlags
        );

        ImGui::PopStyleColor(2);

    }

    void ImGuiSystem::drawConsole()
    {
        ImGui::Begin("Console", nullptr, ImGuiWindowFlags_MenuBar);

        if(ImGui::BeginMenuBar())
        {
            if(ImGui::Button("Clear"))
            {
                consoleMessages.clear();
            }
            ImGui::EndMenuBar();
        }

        std::optional<size_t> pendingDeleteIndex;

        for (size_t i = 0; i < consoleMessages.size(); ++i)
        {
            const auto& message = consoleMessages[i];

            ImGui::PushID(static_cast<int>(i));

            // float wrapWidth = ImGui::GetContentRegionAvail().x;
            // ImVec2 textSize = ImGui::CalcTextSize(msg.c_str(), nullptr, false, wrapWidth);
            // ImVec2 selectablePos = ImGui::GetCursorPos();
            // ImGui::Selectable("##message", false, 0, ImVec2(0.0f, textSize.y));

            auto styledText = AnsiParser::parse(message.formatted);

            const float maxWidth = ImGui::GetContentRegionAvail().x;
            ImDrawList* dl = ImGui::GetWindowDrawList();

            const ImVec2 pos = ImGui::GetCursorScreenPos();
            const float lineHeight = ImGui::GetTextLineHeightWithSpacing();

            float x = 0.0f;
            float y = 0.0f;

            for (const auto& segment : styledText)
            {
                std::string remaining = segment.text;

                while (!remaining.empty())
                {
                    auto split = splitStringByMaxWidth(remaining, maxWidth - x);

                    if (split.first.empty())
                    {
                        x = 0.0f;
                        y += lineHeight;
                        continue;
                    }

                    dl->AddText(
                        ImVec2(pos.x + x, pos.y + y),
                        ImGui::ColorConvertFloat4ToU32(
                            ImVec4(segment.style.fg.x, segment.style.fg.y, segment.style.fg.z, segment.style.fg.w)
                        ),
                        split.first.c_str()
                    );

                    x += ImGui::CalcTextSize(split.first.c_str()).x;
                    remaining = split.second;

                    if (!remaining.empty())
                    {
                        x = 0.0f;
                        y += lineHeight;
                    }
                }
            }


            ImGui::Selectable("##log_message", false, 0 , ImVec2(maxWidth, y + ImGui::GetTextLineHeight()));


            if (ImGui::BeginPopupContextItem("MessageContext"))
            {
                if (ImGui::MenuItem("Copy"))
                {
                    ImGui::SetClipboardText(message.unformatted.c_str());
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

    std::pair<std::string, std::string> ImGuiSystem::splitStringByMaxWidth(const std::string& input, float maxWidth)
    {
        if (input.empty())
        {
            return { "", "" };
        }

        if (maxWidth <= 0.0f)
        {
            return { input.substr(0, 1), input.substr(1) };
        }

        size_t lastGood = 0;

        for (size_t i = 1; i <= input.size(); ++i)
        {
            std::string candidate = input.substr(0, i);
            if (ImGui::CalcTextSize(candidate.c_str()).x > maxWidth)
            {
                break;
            }

            lastGood = i;
        }

        if (lastGood == 0)
        {
            return { input.substr(0, 1), input.substr(1) };
        }

        return { input.substr(0, lastGood), input.substr(lastGood) };
    }

    void ImGuiSystem::event(Event *event)
    {
        if(const auto* rhiWindowEvent = dynamic_cast<RhiWindowEvent*>(event))
        {
            urhi::ImGuiController::processEvent(rhiWindowEvent->event);
        }
    }
}
