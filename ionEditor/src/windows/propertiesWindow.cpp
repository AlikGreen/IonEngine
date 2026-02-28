#include "propertiesWindow.h"

#include "sceneGraphWindow.h"
#include "../editorSystem.h"
#include "../events/inspectEvent.h"
#include "core/engine.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/imGuiSystem.h"
#include "graphics/components/camera.h"
#include "graphics/components/pointLight.h"
#include "imgui/imGuiExtensions.h"
#include "../util/propertyGrid.h"

#include <grl/grl.h>

#include "core/sceneManager.h"

namespace ion::Editor
{
    int levenshteinDistance(const std::string& a, const std::string& b)
    {
        std::string source(a), target(b);
        for (char& c : source) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        for (char& c : target) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        if (source.size() < target.size()) source.swap(target);
        int n = static_cast<int>(source.size()), m = static_cast<int>(target.size());
        std::vector<int> row(m + 1);
        for (int j = 0; j <= m; ++j) row[j] = j;

        for (int i = 1; i <= n; ++i)
        {
            int prevDiag = row[0];
            row[0] = i;
            for (int j = 1; j <= m; ++j)
            {
                const int temp = row[j];
                const int cost = (source[i - 1] == target[j - 1]) ? 0 : 1;
                row[j] = std::min(std::min(temp + 1, row[j - 1] + 1), prevDiag + cost);
                prevDiag = temp;
            }
        }

        return row[m];
    }

    void PropertiesWindow::render()
    {
        ImGui::Begin("Properties");

        if(m_currentInspected.has_value())
        {
            if (const auto entity = std::get_if<entis::Entity>(&m_currentInspected.value()))
            {
                drawEntity(*entity);
            }
            else if (const auto asset = std::get_if<AssetId>(&m_currentInspected.value()))
            {
                drawAsset(*asset);
            }
        }

        ImGui::End();
    }

    void PropertiesWindow::event(Event *event)
    {
        if(const auto* inspectEvent = dynamic_cast<InspectEvent*>(event))
        {
            m_currentInspected = inspectEvent->inspected;
        }
    }


    void PropertiesWindow::drawEntity(const entis::Entity entity)
    {
        const auto& registry = Engine::getSceneManager().getCurrentScene().getRegistry();
        if(!registry.isValid(entity)) return;

        ImGui::Spacing();
        if (entity.has<Tag>())
        {
            drawComponentSpacing();
            drawComponent<Tag>(entity);
        }
        if (entity.has<Transform>())
        {
            drawComponentSpacing();
            drawComponent<Transform>(entity);
        }
        if (entity.has<Camera>())
        {
            drawComponentSpacing();
            drawComponent<Camera>(entity);
        }
        if (entity.has<PointLight>())
        {
            drawComponentSpacing();
            drawComponent<PointLight>(entity);
        }
        if (entity.has<MeshRenderer>())
        {
            drawComponentSpacing();
            drawComponent<MeshRenderer>(entity);
        }

        drawComponentSpacing();

        const float addCompBtnWidth = ImGui::GetContentRegionAvail().x;
        ImGui::PushFont(ImGuiSystem::subheadingFont);
        if (ImGui::Button("Add Component", ImVec2(addCompBtnWidth, 0)))
        {
            ImGui::OpenPopup("componentPopup");
        }
        ImGui::PopFont();

        auto* editorSystem = Engine::getSystem<EditorSystem>();

        const auto popupPos = ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y);
        ImGui::SetNextWindowPos(popupPos);
        ImGui::SetNextWindowSize(ImVec2(addCompBtnWidth, 0.0f), ImGuiCond_Always);

        if (ImGui::BeginPopup("componentPopup"))
        {
            ImGui::InputText("##compSearch", &m_componentSearchString);
            const float compBtnWidth = ImGui::GetContentRegionAvail().x;

            for(const auto& component : editorSystem->getComponents())
            {
                if(!m_componentSearchString.empty() && grl::Str::levenshtein(grl::Str::lower(m_componentSearchString), grl::Str::lower(component.name)) > 3 && !component.name.contains(m_componentSearchString))
                    continue;

                if(ImGui::Button(component.name.c_str(), ImVec2(compBtnWidth, 0)))
                {
                    component.emplaceCallback(entity);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }

    void PropertiesWindow::drawAsset(const AssetId asset)
    {
        AssetManager& assetManager = Engine::getAssetManager();
        ImGui::PushFont(ImGuiSystem::headingFont);
        const AssetMetadata metadata = assetManager.getMetadata(asset);
        ImGui::TextUnformatted(metadata.name.c_str());
        ImGui::PopFont();

        if(metadata.type == typeid(Mesh))
            drawAssetType<Mesh>(asset);
        if(metadata.type == typeid(MaterialShader))
            drawAssetType<MaterialShader>(asset);
    }

    void PropertiesWindow::drawComponentSpacing()
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    void PropertiesWindow::drawComponentTitle(const char* text)
    {
        ImGui::PushFont(ImGuiSystem::headingFont);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImGuiStyle& style = ImGui::GetStyle();

        const ImVec2 startPos = ImGui::GetCursorScreenPos();
        const float width = ImGui::GetContentRegionAvail().x;

        const float paddingX = style.FramePadding.x;
        const float paddingY = style.FramePadding.y;

        const float textHeight = ImGui::GetTextLineHeight();
        const float height = textHeight + paddingY * 2.0f;

        const ImVec2 min = startPos;
        const auto max = ImVec2(startPos.x + width, startPos.y + height);

        drawList->AddRectFilled(min, max, 0x22ffffff, style.FrameRounding);

        ImGui::SetCursorScreenPos(ImVec2(startPos.x + paddingX, startPos.y + paddingY));
        ImGui::TextUnformatted(text);

        ImGui::SetCursorScreenPos(ImVec2(startPos.x, max.y));
        ImGui::PopFont();
        ImGui::Dummy({0.0f, 8.0f});
    }

    template<>
    void PropertiesWindow::drawComponent<Transform>(entis::Entity entity)
    {
        auto& transform = entity.get<Transform>();

        drawComponentTitle("Transform");

        PropertyGrid grid("##TransformGrid");

        glm::vec3 position = transform.position;
        grid.nextRow("Position");
        if (ImGui::DragFloat3("##value", &position.x, 0.05f))
        {
            transform.position = position;
        }
        grid.endRow();

        glm::vec3 rotation = degrees(transform.rotation);
        grid.nextRow("Rotation");
        if (ImGui::DragFloat3("##value", &rotation.x, 0.1f))
        {
            transform.rotation = radians(rotation);
        }
        grid.endRow();

        glm::vec3 scale = transform.scale;
        grid.nextRow("Scale");
        if (ImGui::DragFloat3("##value", &scale.x, 0.025f))
        {
            transform.scale = scale;
        }
        grid.endRow();
    }

    template<>
    void PropertiesWindow::drawComponent<Tag>(entis::Entity entity)
    {
        auto& tag = entity.get<Tag>();

        drawComponentTitle("Tag");

        PropertyGrid grid("##TagGrid");

        grid.nextRow("Name");
        ImGui::InputText("##value", &tag.name);
        grid.endRow();
    }

    template<>
    void PropertiesWindow::drawComponent<Camera>(entis::Entity entity)
    {
        auto& camera = entity.get<Camera>();

        drawComponentTitle("Camera");

        PropertyGrid grid("##CameraGrid");

        grid.nextRow("Background");
        ImGui::ColorEdit4("##value", &camera.bgColor.x);
        grid.endRow();

        float fov = camera.getFov();
        grid.nextRow("FOV");
        if (ImGui::DragFloat("##value", &fov, 0.1f, 1.0f, 179.0f, "%.2f"))
        {
            camera.setFov(fov);
        }
        grid.endRow();

        float nearClip = camera.getNear();
        grid.nextRow("Near");
        if (ImGui::DragFloat("##value", &nearClip, 0.01f, 0.0001f, 10000.0f, "%.4f"))
        {
            camera.setNear(nearClip);
        }
        grid.endRow();

        float farClip = camera.getFar();
        grid.nextRow("Far");
        if (ImGui::DragFloat("##value", &farClip, 1.0f, 0.01f, 1000000.0f, "%.2f"))
        {
            camera.setFar(farClip);
        }
        grid.endRow();

        float aspectRatio = camera.getAspectRatio();
        grid.nextRow("Aspect");
        if (ImGui::DragFloat("##value", &aspectRatio, 0.01f, 1, 16384))
        {
            camera.setAspectRatio(aspectRatio);
        }
        grid.endRow();

    }

    template<>
    void PropertiesWindow::drawComponent<PointLight>(entis::Entity entity)
    {
        auto& pointLight = entity.get<PointLight>();

        drawComponentTitle("Point Light");

        PropertyGrid grid("##PointLightGrid");

        grid.nextRow("Color");
        if (ImGui::ColorEdit3("##value", &pointLight.color.x))
        { }
        grid.endRow();

        grid.nextRow("Power");
        ImGui::DragFloat("##value", &pointLight.power, 0.05f, 0.0f, 100000.0f, "%.2f");
        grid.endRow();
    }

    template<>
    void PropertiesWindow::drawComponent<MeshRenderer>(entis::Entity entity)
    {
        const auto& meshRenderer = entity.get<MeshRenderer>();

        drawComponentTitle("Mesh Renderer");

        PropertyGrid grid("##MeshRendererGrid");

        grid.nextRow("Mesh");
        ImGui::TextUnformatted(meshRenderer.mesh ? "<Assigned>" : "<None>");
        grid.endRow();

        const AssetRef<MaterialShader> material = meshRenderer.getMaterial();

        grid.nextRow("Material");
        ImGui::TextUnformatted(material ? "<Assigned>" : "<None>");
        grid.endRow();
    }

    template<>
    void PropertiesWindow::drawAssetType<Mesh>(const AssetId asset)
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const AssetRef<Mesh> mesh = assetManager.getAsset<Mesh>(asset);

        drawComponentSpacing();

        ImGui::Text("Vertex count: %ld", mesh->getVertexCount());
        ImGui::Text("Index count: %ld", mesh->getIndexCount());
    }

    template<>
    void PropertiesWindow::drawAssetType<MaterialShader>(const AssetId asset)
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const AssetRef<MaterialShader> mat = assetManager.getAsset<MaterialShader>(asset);

        drawComponentSpacing();

        PropertyGrid propGrid("##MaterialPropertiesGrid");
        for(const urhi::ShaderReflection::Member& property : mat->getProperties())
        {
            propGrid.nextRow(property.name.c_str());
            if(property.type == urhi::ShaderReflection::DataType::Float)
            {
                auto val = mat->getProperty<float>(property.name).value();
                ImGui::DragFloat("##float", &val, 0.01f);
                mat->setProperty(property.name, val);
            }
            else if(property.type == urhi::ShaderReflection::DataType::Float2)
            {
                auto vec2 = mat->getProperty<glm::vec2>(property.name).value();
                ImGui::DragFloat2("", &vec2.x, 0.01f);
                mat->setProperty(property.name, vec2);
            }
            else if(property.type == urhi::ShaderReflection::DataType::Float3)
            {
                auto vec3 = mat->getProperty<glm::vec3>(property.name).value();
                ImGui::DragFloat3("", &vec3.x, 0.01f);
                mat->setProperty(property.name, vec3);
            }
            else if(property.type == urhi::ShaderReflection::DataType::Float4)
            {
                auto vec4 = mat->getProperty<glm::vec4>(property.name).value();
                ImGui::DragFloat4("", &vec4.x, 0.01f);
                mat->setProperty(property.name, vec4);
            }
            propGrid.endRow();
        }
    }
}
