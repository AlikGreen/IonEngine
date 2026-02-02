#pragma once
#include "device.h"
#include "editorWindow.h"
#include "../objectPickingPass.h"

#include "graphics/renderer.h"
#include "graphics/components/camera.h"

namespace Neon::Editor
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
    Box<ObjectPickingPass> m_objectPickingPass;

    Rc<RHI::Device> m_device;
    Rc<RHI::Buffer> m_matrixUniformBuffer;
    Rc<RHI::Buffer> m_cameraUniformBuffer;
    Rc<RHI::Buffer> m_pointLightsUniformBuffer;


    Rc<RHI::Sampler> m_gizmoSampler;
    Rc<RHI::TextureView> m_lightTextureView{};

    Camera m_objPickerCamera{};
    Rc<RHI::TextureView> m_objPickerTexView{};

    ECS::Entity selectedEntity = ECS::Entity::null();

    bool m_wasFocused = false;
    bool m_mouseOverViewport = false;
};
}
