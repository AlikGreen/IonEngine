#include "statisticsWindow.h"

#include <imgui.h>

#include "core/engine.h"

namespace ion::Editor
{
    void StatisticsWindow::render()
    {
        ImGui::Begin("Statistics");

        ImGui::Text(fmt::format("Frame time: {:.2f} ms", Engine::getDeltaTime()*1000).c_str());
        ImGui::Text(fmt::format("FPS: {:.0f}", 1.0f/Engine::getDeltaTime()).c_str());

        ImGui::End();

    }
}
