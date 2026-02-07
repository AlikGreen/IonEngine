#include "editorViewportWindow.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include "../editorCamera.h"
#include "../objectPickingPass.h"
#include "../events/inspectEvent.h"
#include "core/engine.h"
#include "core/eventManager.h"
#include "core/sceneManager.h"
#include "core/components/transformComponent.h"
#include "graphics/graphicsSystem.h"
#include "graphics/image.h"
#include "graphics/components/camera.h"
#include "graphics/components/pointLight.h"
#include "graphics/events/windowResizeEvent.h"
#include "graphics/renderPasses/opaqueForwardScenePass.h"
#include "graphics/renderPasses/skyboxPass.h"
#include "imgui/imGuiExtensions.h"
#include "input/input.h"

namespace ion::Editor
{
    struct MatrixUniforms
    {
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
        glm::mat4 modelMatrix;
    };


    void EditorViewportWindow::startup()
    {
        m_device = Engine::getSystem<GraphicsSystem>()->getDevice();

        AssetManager& assetManager = Engine::getAssetManager();
        const auto texData = assetManager.import<TextureData>("textures/light-bulb.png");
        const auto image = Image(texData);
        const auto viewDesc = urhi::TextureViewDesc(image.texture);
        m_lightTextureView = m_device->createTextureView(viewDesc);
        m_gizmoSampler = image.sampler;

        m_matrixUniformBuffer = m_device->createUniformBuffer();
        m_cameraUniformBuffer = m_device->createUniformBuffer();
        m_pointLightsUniformBuffer = m_device->createUniformBuffer();

        m_billboardMaterial = MaterialShader::createBillboard();

        const auto cl = m_device->createCommandList();
        cl->begin();
        cl->reserveBuffer(m_matrixUniformBuffer, sizeof(MatrixUniforms));
        cl->reserveBuffer(m_cameraUniformBuffer, sizeof(CameraUniformData));
        cl->reserveBuffer(m_pointLightsUniformBuffer, sizeof(PointLightsUniformData));
        m_device->submit(cl);

        m_renderer.addPass<ForwardSceneRenderPass>(m_device);
        m_renderer.addPass<SkyboxRenderPass>();

        m_objectPickingPass = grl::makeBox<ObjectPickingPass>(m_device);
    }

    void EditorViewportWindow::update()
    {
        auto& registry = Engine::getSceneManager().getCurrentScene().getRegistry();
        const auto& cameras = registry.view<Camera, EditorCamera, Transform>();

        if(cameras.empty())
            return;

        auto [camEntity, camera, editorCamera, camTransform] = cameras.at(0);

        if(Input::isButtonHeld(MouseButton::Right) && m_mouseOverViewport)
        {
            m_wasFocused = true;
            constexpr float sens = 5.0f; // In future get from settings
            float xDelta = Input::getMouseDelta().x * Engine::getDeltaTime() * sens;
            camTransform.rotate({ 0.0f, xDelta, 0.0f });
            float yDelta = Input::getMouseDelta().y * Engine::getDeltaTime() * sens;
            camTransform.rotate({ yDelta, 0.0f, 0.0f });

            constexpr float camSpeed = 5.0f; // In future get from settings

            if(Input::isKeyHeld(KeyCode::W))
                camTransform.translate(camTransform.forward() * Engine::getDeltaTime() * camSpeed);
            if(Input::isKeyHeld(KeyCode::S))
                camTransform.translate(camTransform.backward() * Engine::getDeltaTime() * camSpeed);
            if(Input::isKeyHeld(KeyCode::A))
                camTransform.translate(camTransform.left() * Engine::getDeltaTime() * camSpeed);
            if(Input::isKeyHeld(KeyCode::D))
                camTransform.translate(camTransform.right() * Engine::getDeltaTime() * camSpeed);
            if(Input::isKeyHeld(KeyCode::Space))
                camTransform.translate(camTransform.up() * Engine::getDeltaTime() * camSpeed);
            if(Input::isKeyHeld(KeyCode::LShift))
                camTransform.translate(camTransform.down() * Engine::getDeltaTime() * camSpeed);

            Input::setCursorLocked(true);
            Input::setCursorVisible(false);
        }else if(m_wasFocused)
        {
            m_wasFocused = false;

            Input::setCursorLocked(false);
            Input::setCursorVisible(true);
        }
    }

    void drawGizmo(const glm::mat4& view, const glm::mat4& projection, glm::mat4& model, ImVec2 windowPos, ImVec2 windowSize)
    {
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        // Use the actual screen-space coordinates of the viewport image
        ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);

        static ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
        static ImGuizmo::MODE mode = ImGuizmo::WORLD;

        // Add hotkeys for convenience while we're here
        if (ImGui::IsKeyPressed(ImGuiKey_G)) op = ImGuizmo::TRANSLATE; // G for Grab
        if (ImGui::IsKeyPressed(ImGuiKey_R)) op = ImGuizmo::ROTATE;    // R for Rotate
        if (ImGui::IsKeyPressed(ImGuiKey_S)) op = ImGuizmo::SCALE;     // S for Scale

        ImGuizmo::Manipulate(&view[0][0], &projection[0][0], op, mode, &model[0][0]);
    }

    void EditorViewportWindow::render()
    {
        ImGui::Begin("Viewport");
        auto& scene = Engine::getSceneManager().getCurrentScene();
        auto& registry = scene.getRegistry();
        const auto& cameras = registry.view<Camera, EditorCamera, Transform>();

        const ImVec2 avail = ImGui::GetContentRegionAvail();

        if(cameras.empty())
        {
            ImGui::End();
            return;
        }

        auto [camEntity, camera, editorCamera, camTransform] = cameras.at(0);
        if(camera.renderTarget != nullptr)
        {
            camera.setAspectRatio(static_cast<float>(avail.x) / static_cast<float>(avail.y));
            camera.renderTarget->resize(static_cast<uint32_t>(avail.x), static_cast<uint32_t>(avail.y)); // Checks if dimensions are the same before resizing

            RenderContext renderCtx{};

            CulledRenderables cullingResult = Renderer::performCulling(scene, camEntity);
            CameraUniformData cameraUniformData = Renderer::getCameraUniformData(camEntity);
            PointLightsUniformData pointLightsUniformData = Renderer::getPointLightsUniformData(scene);

            renderCtx.set("renderables", &cullingResult.all);
            renderCtx.set("skybox_material", camera.skyboxMaterial.get());
            renderCtx.set("scene_color_texture", camera.renderTarget->getColorAttachment());
            renderCtx.set("scene_depth_texture", camera.renderTarget->getDepthAttachment());
            renderCtx.set("camera_buffer", m_cameraUniformBuffer);
            renderCtx.set("point_lights_buffer", m_pointLightsUniformBuffer);


            auto cmd = m_device->createCommandList();
            cmd->begin();

            cmd->updateBuffer(m_cameraUniformBuffer, cameraUniformData);
            cmd->updateBuffer(m_pointLightsUniformBuffer, pointLightsUniformData);

            m_renderer.execute(cmd, renderCtx);

            m_device->submit(cmd);

            // renderBillboards(camEntity, camTransform, camera);
            const grl::Rc<urhi::TextureView> sceneTexture = camera.renderTarget->getColorAttachment();
            NeonGui::Image(sceneTexture, avail, ImVec2(0, 1), ImVec2(1, 0));

            ImVec2 windowPos = ImGui::GetItemRectMin();
            ImVec2 windowSize = ImGui::GetItemRectSize();

            m_mouseOverViewport = ImGui::IsItemHovered();

            if(selectedEntity.isValid())
            {
                glm::mat4 modelMat = selectedEntity.get<Transform>().getLocalMatrix();
                drawGizmo(cameraUniformData.view, camera.getProjectionMatrix(), modelMat, windowPos, windowSize);
                selectedEntity.get<Transform>().setLocalMatrix(modelMat);
            }

            if(ImGui::IsItemClicked())
            {
                updatePickerTexture(static_cast<uint32_t>(avail.x), static_cast<uint32_t>(avail.y));
                renderCtx.set("picker_color_texture", m_objPickerTexView);
                renderCtx.set("picker_depth_texture", camera.renderTarget->getDepthAttachment());
                cmd = m_device->createCommandList();
                cmd->begin();
                m_objectPickingPass->execute(cmd, renderCtx);
                m_device->submit(cmd);
                const ImVec2 rectMin = ImGui::GetItemRectMin();
                const ImVec2 mouse = ImGui::GetIO().MousePos;
                ImVec2 rectMax = ImGui::GetItemRectMax();
                const auto relative = ImVec2(mouse.x - rectMin.x, rectMax.y - mouse.y);

                std::vector<uint32_t> data{};
                data.resize(1);

                const auto cl = m_device->createCommandList();
                cl->begin();

                urhi::TextureReadDesc readDesc{};
                readDesc.width = 1;
                readDesc.height = 1;
                readDesc.depth = 1;
                readDesc.x = static_cast<uint32_t>(relative.x);
                readDesc.y = static_cast<uint32_t>(relative.y);
                readDesc.pixelLayout = urhi::PixelLayout::RInt;
                readDesc.pixelType = urhi::PixelType::UnsignedInt;

                cl->readTexture(m_objPickerTexView, readDesc, data);
                m_device->submit(cl);

                if(data[0] != 0)
                {
                    Engine::getEventManager().queueEvent<InspectEvent>(registry.getEntity(data[0]));
                }
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_ID"))
                {
                    AssetManager& assetManager = Engine::getAssetManager();
                    Scene& currentScene = Engine::getSceneManager().getCurrentScene();
                    const AssetId droppedID = *static_cast<AssetId*>(payload->Data);

                    if(assetManager.assetIsType<Scene>(droppedID))
                    {
                        const AssetRef<Scene> prefab = assetManager.getAsset<Scene>(droppedID);
                        currentScene.import(*prefab);
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }

        ImGui::End();
    }

    void EditorViewportWindow::event(Event *event)
    {
        if(const auto* inspectEvent = dynamic_cast<InspectEvent*>(event))
        {
            if (const auto entity = std::get_if<entis::Entity>(&inspectEvent->inspected))
            {
                selectedEntity = *entity;
            }
        }
    }

    void EditorViewportWindow::updatePickerTexture(const uint32_t width, const uint32_t height)
    {
        if(m_objPickerTexView != nullptr && width == m_objPickerTexView->getWidth() && height == m_objPickerTexView->getHeight())
            return;

        const urhi::TextureDesc texDesc = urhi::TextureDesc::Texture2D(width, height, urhi::PixelFormat::R8Uint);
        const grl::Rc<urhi::Texture> pickerTex = m_device->createTexture(texDesc);

        const auto viewDesc = urhi::TextureViewDesc(pickerTex);
        m_objPickerTexView = m_device->createTextureView(viewDesc);
    }
}
    // void EditorViewportWindow::renderBillboards(ECS::Entity camEntity, const Transform& camTransform, Camera& camera)
    // {
    //     auto& registry = Engine::getSceneManager().getCurrentScene().getRegistry();
    //
    //     const auto cl = m_device->createCommandList();
    //
    //     updateFramebuffer(camera.renderTarget);
    //
    //     cl->begin();
    //     cl->setPipeline(m_billboardMaterial.getPipeline());
    //     cl->setFramebuffer(m_framebuffer);
    //
    //     const auto& pointLights = registry.view<PointLight, Transform>();
    //
    //     const glm::mat4 flip = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));
    //     const glm::mat4 viewMatrix = glm::inverse(Transform::getWorldMatrix(camEntity) * flip);
    //
    //     MatrixUniforms matrixUniforms{};
    //     matrixUniforms.projMatrix = camera.getProjectionMatrix();
    //     matrixUniforms.viewMatrix = viewMatrix;
    //
    //     for(auto [entity, pointLight, transform] : pointLights)
    //     {
    //         matrixUniforms.modelMatrix = glm::inverse(glm::lookAt(transform.getPosition(), camTransform.getPosition(), transform.up()));
    //             // calculateBillboardMatrix(transform.getPosition(), matrixUniforms.viewMatrix, matrixUniforms.projMatrix);
    //
    //         cl->updateBuffer(m_matrixUniformBuffer, matrixUniforms);
    //         cl->setUniformBuffer("MatrixUniforms", m_matrixUniformBuffer);
    //         cl->setSampler("gizmoSampler", m_gizmoSampler);
    //         cl->setTexture("gizmoTexture", m_lightTextureView);
    //
    //         cl->setVertexBuffer(0, m_screenMesh.getVertexBuffer());
    //         cl->setIndexBuffer(m_screenMesh.getIndexBuffer(), RHI::IndexFormat::UInt32);
    //
    //         cl->drawIndexed(6);
    //     }
    //
    //     m_device->submit(cl);
    // }
// }
