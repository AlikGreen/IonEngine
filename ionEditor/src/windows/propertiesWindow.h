#pragma once
#include <imgui.h>
#include <optional>

#include "editorWindow.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/components/meshRenderer.h"
#include "graphics/components/pointLight.h"


namespace ion::Editor
{
class PropertiesWindow final : public EditorWindow
{
public:
    void render() override;
    void event(Event *event) override;
private:
    enum class LastSelected { None, Asset, Entity };
    void drawEntity(entis::Entity entity);
    void drawAsset(AssetId asset);

    template<typename T>
    void drawAssetType(AssetId asset)
    {

    }

    template<typename T>
    void drawComponent(const entis::Entity entity)
    {
        if(entity.has<T>())
            ImGui::Text(typeid(T).name());
    }

    static void drawComponentSpacing();
    static void drawComponentTitle(const char *text);

    LastSelected m_latestViewedType = LastSelected::None;
    std::string m_renameString{};
    std::string m_componentSearchString{};
    std::optional<std::variant<entis::Entity, AssetId>> m_currentInspected;
};

template<> void PropertiesWindow::drawComponent<Transform>(entis::Entity entity);
template<> void PropertiesWindow::drawComponent<Tag>(entis::Entity entity);
template<> void PropertiesWindow::drawComponent<Camera>(entis::Entity entity);
template<> void PropertiesWindow::drawComponent<PointLight>(entis::Entity entity);
template<> void PropertiesWindow::drawComponent<MeshRenderer>(entis::Entity entity);

template<> void PropertiesWindow::drawAssetType<Mesh>(AssetId asset);
template<> void PropertiesWindow::drawAssetType<MaterialShader>(AssetId asset);
}
