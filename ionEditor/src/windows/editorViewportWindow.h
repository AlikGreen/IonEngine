#pragma once
#include "device.h"
#include "editorWindow.h"
#include "../objectPickingPass.h"

#include "graphics/renderer.h"
#include "graphics/components/camera.h"

namespace ion::Editor
{
class EditorViewportWindow final : public EditorWindow
{
public:
    void startup() override;
    void update()  override;
    void render()  override;

    void event(Event *event) override;
private:
    void updatePickerTexture(uint32_t width, uint32_t height);
    MaterialShader m_billboardMaterial{};
    Renderer m_renderer;
    grl::Box<ObjectPickingPass> m_objectPickingPass;

    grl::Rc<urhi::Device> m_device;
    grl::Rc<urhi::Buffer> m_matrixUniformBuffer;
    grl::Rc<urhi::Buffer> m_cameraUniformBuffer;
    grl::Rc<urhi::Buffer> m_pointLightsUniformBuffer;


    grl::Rc<urhi::Sampler> m_gizmoSampler;
    grl::Rc<urhi::TextureView> m_lightTextureView{};

    Camera m_objPickerCamera{};
    grl::Rc<urhi::TextureView> m_objPickerTexView{};

    entis::Entity selectedEntity = entis::Entity::null();

    bool m_wasFocused = false;
    bool m_mouseOverViewport = false;
};
}
