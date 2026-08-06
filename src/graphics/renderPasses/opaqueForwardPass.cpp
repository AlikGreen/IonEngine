#include "opaqueForwardPass.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/sceneRenderer/gpuMaterialRegistry.h"
#include "graphics/sceneRenderer/gpuSceneBuffers.h"
#include "graphics/sceneRenderer/meshletFrameResources.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    struct ModelUniforms
    {
        glm::mat4 model{};
        glm::mat4 normalMatrix{};
    };

    OpaqueForwardPass::OpaqueForwardPass(const dg::Ref<dg::IRenderDevice>& device)
    {
        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        m_shaderModule = importPipeline.import<ShaderModule>("shaders/genericOpaqueForward.hlsl");
    }

    void OpaqueForwardPass::execute(const dg::Ref<dg::IDeviceContext>& dc, RenderContext &ctx)
    {
        if(!ctx.has("camera_buffer")
            || !ctx.has("material_registry")
            || !ctx.has("scene_buffers")
            || !ctx.has("frame_resources")
            || !ctx.has("point_lights_buffer")
            || !ctx.has("scene_rtv")
            || !ctx.has("scene_dtv")
            || !ctx.has("pass_data_buffer"))
        {
            return;
        }

        const auto cameraBuffer = ctx.get<dg::Ref<dg::IBuffer>>("camera_buffer");
        const auto pointLightsBuffer = ctx.get<dg::Ref<dg::IBuffer>>("point_lights_buffer");
        const auto passDataBuffer = ctx.get<dg::Ref<dg::IBuffer>>("pass_data_buffer");

        const auto sceneColorRTV = ctx.get<dg::Ref<dg::ITextureView>>("scene_rtv");
        const auto sceneDepthDSV = ctx.get<dg::Ref<dg::ITextureView>>("scene_dtv");

        const auto& sceneBuffers = *ctx.get<GpuSceneBuffers*>("scene_buffers");
        const auto& frameResources = *ctx.get<MeshletFrameResources*>("frame_resources");
        const auto& materialRegistry = *ctx.get<GpuMaterialRegistry*>("material_registry");

        auto vertexBuffer = sceneBuffers.vertexBuffer();
        auto transformBuffer = sceneBuffers.transformBuffer();
        auto indexBuffer = frameResources.indexScratchBuffer;
        auto drawCmdsBuffer = frameResources.drawCmdBuffer;
        auto drawCountsBuffer = frameResources.drawCountBuffer;


        PassDefinition passDef;
        passDef.name = "opaque_forward_pass";
        passDef.topology = dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        passDef.rtvFormats.push_back(dg::TEX_FORMAT_RGBA8_UNORM);
        passDef.dtvFormat = dg::TEX_FORMAT_D32_FLOAT;

        passDef.overrides.cullMode = dg::CULL_MODE_NONE; // TODO set to back once working
        passDef.overrides.depth = DepthPreset::ReadWrite;
        passDef.overrides.blend = BlendPreset::Opaque;



        dg::ITextureView* pRTVs[] = { sceneColorRTV };
        dc->SetRenderTargets(1, pRTVs, sceneDepthDSV, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        constexpr float clearColor[] = { 0.f, 0.f, 0.f, 1.f };
        dc->ClearRenderTarget(sceneColorRTV, clearColor, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        dc->ClearDepthStencil(sceneDepthDSV, dg::CLEAR_DEPTH_FLAG, 1.0f, 0, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        uint32_t drawBufferIndex = 0;
        for(uint32_t i = 0; i < materialRegistry.templates().size(); i++)
        {
            auto [pso, srb] = materialRegistry.templates().at(i)->getOrCreatePipeline(*m_shaderModule, passDef);
            dc->SetPipelineState(pso);

            srb->GetVariableByName(dg::SHADER_TYPE_VERTEX, "camera")->Set(cameraBuffer);
            srb->GetVariableByName(dg::SHADER_TYPE_VERTEX, "modelData")->Set(transformBuffer);
            srb->GetVariableByName(dg::SHADER_TYPE_PIXEL, "pass")->Set(passDataBuffer);
            srb->GetVariableByName(dg::SHADER_TYPE_PIXEL, "pointLights")->Set(pointLightsBuffer);

            // pass.pushConstants(modelUniforms); push constants needs to have index into global buffer
            // renderable.material->applyBindings(cmd, pass); need to add bindless textures and a material buffer

            dc->CommitShaderResources(srb, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            dc->SetVertexBuffers(0, 1, &vertexBuffer, nullptr, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            dc->SetIndexBuffer(indexBuffer, 0, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            dg::DrawIndexedIndirectAttribs drawAttribs{};
            drawAttribs.pAttribsBuffer = drawCmdsBuffer;
            drawAttribs.pCounterBuffer = drawCountsBuffer;
            drawAttribs.IndexType = dg::VT_INT32;
            drawAttribs.DrawArgsOffset = drawBufferIndex * sizeof(DrawIndexedIndirectCommand);
            drawAttribs.CounterOffset = i * sizeof(uint32_t);
            drawAttribs.DrawCount = 2048;

            dc->DrawIndexedIndirect(drawAttribs);

            const auto templInfo = materialRegistry.templateInfos().at(i);
            drawBufferIndex += templInfo.totalPrimitiveCount;
        }
    }
}
