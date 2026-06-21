#include "rendererSystem.h"

#include "DrawIndexedIndirectCommand.h"
#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "core/components/transformComponent.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    void RendererSystem::startup()
    {
        m_device = Engine::getSystem<GraphicsSystem>()->getDevice();
        m_pipeline = grl::makeBox<IndirectMeshletPipeline>(m_device);
        m_sceneBuffers = grl::makeBox<GpuSceneBuffers>(m_device);
        m_matRegistry = grl::makeBox<GpuMaterialRegistry>(m_device);
    }

    void RendererSystem::queueView(const grl::Rc<Renderer>& renderer, const grl::Rc<RenderContext>& ctx, Camera camera, glm::mat4 cameraTransform)
    {
        m_queuedViews.emplace_back(renderer, ctx, camera, cameraTransform);
    }

    void RendererSystem::update(Scene &scene)
    {
        const auto& meshRenderers = scene.registry().view<MeshRenderer, Transform>();

        const auto cmd = m_device->acquireCommandList(urhi::QueueType::Compute);
        cmd->begin();

        bool meshLoaded = false;


        for (auto[entity, meshRenderer, transform] : meshRenderers)
        {
            if(meshRenderer.mesh == nullptr)
            {
                continue;
            }

            if(needsUnload(*meshRenderer.mesh, transform))
            {
                unloadMesh(*meshRenderer.mesh, cmd);
            }
            else if(needsLoad(entity))
            {
                meshLoaded = true;
                loadMesh(entity, meshRenderer, cmd);
            }else if(transform.dirty())
            {
                m_sceneBuffers->updateTransform(entity, cmd);
            }
        }

        if(meshLoaded)
            m_matRegistry->syncLayout(cmd);

        m_device->submit(cmd);
    }

    void RendererSystem::render(Scene &scene)
    {
        const auto cmd = m_device->acquireCommandList(urhi::QueueType::Compute);
        cmd->begin();

        for(const auto& view : m_queuedViews)
        {

            if(m_sceneBuffers->meshletInstanceCount() > 0)
                m_pipeline->render(cmd, view.camTransform, view.camera, *m_sceneBuffers, *m_matRegistry);

            view.ctx->set("frame_resources", &m_pipeline->frameResources());
            view.ctx->set("scene_buffers", m_sceneBuffers.get());
            view.ctx->set("material_registry", m_matRegistry.get());

            view.renderer->execute(cmd, *view.ctx);

            const auto sceneColorTexture = view.ctx->get<grl::Rc<urhi::TextureView>>("scene_color_texture");
            const auto sceneDepthTexture = view.ctx->get<grl::Rc<urhi::TextureView>>("scene_depth_texture");

            urhi::ColorAttachment colorAttachment{};
            colorAttachment.target = sceneColorTexture;
            colorAttachment.loadOp = urhi::LoadOp::Load;
            colorAttachment.storeOp = urhi::StoreOp::Store;

            urhi::DepthStencilAttachment depthAttachment{};
            depthAttachment.target = sceneDepthTexture;
            depthAttachment.loadOp = urhi::LoadOp::Load;
            depthAttachment.storeOp = urhi::StoreOp::DontCare;

            urhi::RenderPassDesc renderPassDesc{};
            renderPassDesc.colorAttachments = {colorAttachment};
            renderPassDesc.depthAttachment = depthAttachment;

            // auto& renderPass = cmd->beginRenderPass(renderPassDesc);
            //
            // renderPass.setPipeline(m_debugLinesPipeline);
            //
            // renderPass.setVertexBuffer(0, m_pipeline->frameResources().debugLinesBuffer);
            //
            // renderPass.multiDrawIndirect(m_pipeline->frameResources().linesDrawCmdBuffer, 1);
            //
            // renderPass.end();
        }

        m_device->submit(cmd);

        m_queuedViews.clear();
    }

    bool RendererSystem::needsUnload(Mesh &mesh, Transform &transform)
    {
        return false;
    }

    bool RendererSystem::needsLoad(const entis::Entity entity)
    {
        return !m_loadedEntities.contains(entity);
    }

    void RendererSystem::unloadMesh(Mesh &mesh, const grl::Rc<urhi::CommandList> &cmd)
    {
        // TODO implement
    }

    void RendererSystem::loadMesh(const entis::Entity entity, const MeshRenderer& meshRenderer, const grl::Rc<urhi::CommandList> &cmd)
    {
        m_loadedEntities.emplace(entity);

        m_sceneBuffers->upload(entity, meshRenderer, *m_matRegistry, cmd);
    }
}
