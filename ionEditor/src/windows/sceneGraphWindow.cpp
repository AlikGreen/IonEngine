#include "sceneGraphWindow.h"

#include <imgui.h>

#include "propertiesWindow.h"
#include "../editorCamera.h"
#include "core/engine.h"
#include "core/sceneManager.h"
#include "core/components/parentComponent.h"
#include "core/components/tagComponent.h"
#include "graphics/components/camera.h"
#include "graphics/components/pointLight.h"
#include "imgui/imGuiExtensions.h"
#include "../editorSystem.h"
#include "core/eventManager.h"
#include "../events/inspectEvent.h"

namespace ion::Editor
{
    void SceneGraphWindow::update()
    {
        Scene& scene = Engine::getSceneManager().getCurrentScene();
        if(m_pendingDelete.has_value())
        {
            scene.getRegistry().destroy(m_pendingDelete.value());
            m_pendingDelete = std::nullopt;
        }
    }

    void SceneGraphWindow::render()
    {
        ImGui::Begin("Scene Graph");
        Scene& scene = Engine::getSceneManager().getCurrentScene();

        const float lineHeight = ImGui::GetFrameHeight();
        if (ImGui::Button("+", ImVec2(lineHeight, lineHeight)))
        {
            ImGui::OpenPopup("AddEntityPopup");
        }

        if (ImGui::BeginPopup("AddEntityPopup"))
        {
            if (ImGui::MenuItem("Entity"))
            {
                scene.createEntity("New Entity");
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Camera"))
            {
                entis::Entity entity = scene.createEntity("New Camera");
                entity.emplace<Camera>();
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Point Light"))
            {
                entis::Entity entity = scene.createEntity("New Point Light");
                entity.emplace<PointLight>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##search", "Search...", &search);

        buildChildrenMap();

        for (const auto entity : m_rootEntities)
        {
            drawEntityNode(entity);
        }

        ImGui::End();
    }

    void SceneGraphWindow::buildChildrenMap()
    {
        m_childrenMap.clear();
        m_rootEntities.clear();

        auto& registry = Engine::getSceneManager().getCurrentScene().getRegistry();

        for (auto [e, p] : registry.view<Parent>())
        {
            if (!p.hasParent())
                m_rootEntities.push_back(e);
            else
                m_childrenMap[p.getParent()].push_back(e);
        }
    }

    std::string toLower(const std::string& str)
    {
        std::string result = str;

        std::ranges::transform(
            result,
            result.begin(),
            [](const unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });

        return result;
    }

    void SceneGraphWindow::drawEntityNode(const entis::Entity e)
    {
        if(e.has<EditorCamera>()) return;

        auto& world = Engine::getSceneManager().getCurrentScene().getRegistry();

        const auto it = m_childrenMap.find(e);
        const bool hasChildren = (it != m_childrenMap.end() && !it->second.empty());

        const std::string label = "⬡ " + world.get<Tag>(e).name;

        const bool searched = !search.empty() && toLower(world.get<Tag>(e).name).contains(toLower(search));

        ImGui::PushID(static_cast<int>(e.id()));

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (!hasChildren)
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        ImVec2 framePadding = ImGui::GetStyle().FramePadding;
        framePadding.x = 2.0f;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);

        if(searched)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.392, 0.478, 0.729, 1.0f));

        const bool open = ImGui::TreeNodeEx("##node", flags, "%s", label.c_str());

        if(searched)
            ImGui::PopStyleColor();

        ImGui::PopStyleVar();


        if (ImGui::BeginPopupContextItem("EntityContext"))
        {
            if (ImGui::MenuItem("Delete"))
            {
                m_pendingDelete = e;
            }

            ImGui::EndPopup();
        }
        else if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            Engine::getEventManager().queueEvent<InspectEvent>(e);

        if (hasChildren)
        {
            if (open)
            {
                for (const entis::Entity c : it->second)
                {
                    drawEntityNode(c);
                }
                ImGui::TreePop();
            }
        }

        ImGui::PopID();
    }
}
