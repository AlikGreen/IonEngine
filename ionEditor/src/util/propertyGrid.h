#pragma once
#include <imgui.h>

namespace ion::Editor
{
    class PropertyGrid
    {
    public:
        explicit PropertyGrid(const char* id,
            ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp
                | ImGuiTableFlags_BordersInnerV)
            : m_open(ImGui::BeginTable(id, 2, flags))
        {
            if (m_open)
            {
                ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);
            }
        }

        ~PropertyGrid()
        {
            if (m_open)
            {
                ImGui::EndTable();
            }
        }

        PropertyGrid(const PropertyGrid&) = delete;
        PropertyGrid& operator=(const PropertyGrid&) = delete;

        [[nodiscard]] bool isOpen() const
        {
            return m_open;
        }

        void nextRow(const char* label)
        {
            if (!m_open)
            {
                return;
            }

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(label);

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);

            ImGui::PushID(label);
            m_rowOpen = true;
        }

        void endRow()
        {
            if (!m_open || !m_rowOpen)
            {
                return;
            }

            ImGui::PopID();
            m_rowOpen = false;
        }

    private:
        bool m_open = false;
        bool m_rowOpen = false;
    };
}
