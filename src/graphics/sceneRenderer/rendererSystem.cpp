#include "rendererSystem.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "core/components/transformComponent.h"
#include "graphics/graphicsSystem.h"
#include "graphics/pipelineRegistry.h"
#include "graphics/shaders/shaderModule.h"

namespace ion
{
    void RendererSystem::startup()
    {
        const auto graphicsSystem = Engine::getSystem<GraphicsSystem>();
        m_device = graphicsSystem->device();

        m_sceneBuffers = grl::makeBox<GpuSceneBuffers>(m_device);
        m_matRegistry = grl::makeBox<GpuMaterialRegistry>(m_device);

        m_pipeline = grl::makeBox<IndirectMeshletPipeline>(m_device, *m_sceneBuffers, *m_matRegistry);

        auto& importPipeline = Engine::assetImportPipeline();

        const PassDefinition debugLinesPass
        {
            .name       = "debug_bounding_box_lines",
            .rtvFormats = {dg::TEX_FORMAT_RGBA8_UNORM},
            .topology   = dg::PRIMITIVE_TOPOLOGY_LINE_LIST
        };

        const auto module = importPipeline.load<ShaderModule>("shaders/linesDraw.hlsl");

        const auto shaderBundle = graphicsSystem->shaderRegistry().getOrCreate(*module);
        m_debugLinesPSO = graphicsSystem->pipelineRegistry().getOrCreateGraphics(shaderBundle, debugLinesPass);

        m_debugLinesPSO->CreateShaderResourceBinding(&m_debugLinesSRB);
    }

    void RendererSystem::queueView(const grl::Rc<Renderer>& renderer, const grl::Rc<RenderContext>& ctx, const Camera& camera, glm::mat4 cameraTransform)
    {
        m_queuedViews.emplace_back(renderer, ctx, camera, cameraTransform);
    }

    void RendererSystem::update(Scene &scene)
    {
        const auto& meshRenderers = scene.registry().view<MeshRenderer, Transform>();

        const auto cmd = Engine::getSystem<GraphicsSystem>()->imContext();

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
    }

    void RendererSystem::render(Scene &scene)
    {
        const auto dc = Engine::getSystem<GraphicsSystem>()->imContext();

        for(const auto& view : m_queuedViews)
        {

            if(m_sceneBuffers->meshletInstanceCount() > 0)
                m_pipeline->render(dc, view.camTransform, view.camera, *m_sceneBuffers, *m_matRegistry);

            view.ctx->set("frame_resources", &m_pipeline->frameResources());
            view.ctx->set("scene_buffers", m_sceneBuffers.get());
            view.ctx->set("material_registry", m_matRegistry.get());

            view.renderer->execute(dc, *view.ctx);

            auto renderTarget = view.ctx->get<RenderTarget>("scene_rtv");
            const auto cameraBuffer = view.ctx->get<dg::Ref<dg::IBuffer>>("camera_buffer");
            m_debugLinesSRB->GetVariableByName(dg::SHADER_TYPE_VERTEX, "gCamera")->Set(cameraBuffer);

            dc->SetRenderTargets(
                1,
                &renderTarget.getColorRTV(),
                nullptr,
                dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            dc->SetPipelineState(m_debugLinesPSO);

            dc->SetVertexBuffers(0, 1, &m_pipeline->frameResources().debugLinesBuffer, nullptr, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


            dc->CommitShaderResources(m_debugLinesSRB, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            dg::DrawIndirectAttribs drawAttribs{};
            drawAttribs.pAttribsBuffer = m_pipeline->frameResources().linesDrawCmdBuffer;
            drawAttribs.DrawCount = 1;
            drawAttribs.AttribsBufferStateTransitionMode = dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

            dc->DrawIndirect(drawAttribs);
        }

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

    void RendererSystem::unloadMesh(Mesh &mesh, const dg::Ref<dg::IDeviceContext> &cmd)
    {
        // TODO implement
    }

    void RendererSystem::loadMesh(const entis::Entity entity, const MeshRenderer& meshRenderer, const dg::Ref<dg::IDeviceContext> &cmd)
    {
        m_loadedEntities.emplace(entity);

        m_sceneBuffers->upload(entity, meshRenderer, *m_matRegistry, cmd);
    }
}
