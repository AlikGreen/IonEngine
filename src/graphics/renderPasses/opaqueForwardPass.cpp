#include "opaqueForwardPass.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/sceneRenderer/gpuMaterialRegistry.h"
#include "graphics/sceneRenderer/gpuSceneBuffers.h"
#include "graphics/sceneRenderer/meshletFrameResources.h"

namespace ion
{
    struct ModelUniforms
    {
        glm::mat4 model{};
        glm::mat4 normalMatrix{};
    };

    OpaqueForwardPass::OpaqueForwardPass()
    {
        urhi::DepthState depthState{};
        depthState.hasDepthTarget   = true;
        depthState.enableDepthTest  = true;
        depthState.enableDepthWrite = true;

        urhi::RasterizerState rasterState{};
        rasterState.cullMode = urhi::CullMode::Back;

        urhi::BlendState blendState{};
        blendState.enableBlend    = false;

        urhi::ColorAttachmentDesc colorAttach{};
        colorAttach.blend  = blendState;
        colorAttach.format = urhi::PixelFormat::RGBA8UNorm;

        m_pipelineDesc.depthState       = depthState;
        m_pipelineDesc.rasterizerState  = rasterState;
        m_pipelineDesc.colorAttachments = { colorAttach };

        m_pipelineDesc.depthAttachmentFormat = urhi::PixelFormat::Depth32Float;

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        m_shaderModule = importPipeline.import<urhi::slang::Module>("shaders/genericOpaqueForward.slang");
    }

    void OpaqueForwardPass::execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx)
    {
        if(!ctx.has("camera_buffer")
            || !ctx.has("material_registry")
            || !ctx.has("scene_buffers")
            || !ctx.has("frame_resources")
            || !ctx.has("point_lights_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture")
            || !ctx.has("pass_data_buffer"))
        {
            return;
        }

        const auto cameraBuffer = ctx.get<grl::Rc<urhi::Buffer>>("camera_buffer");
        const auto pointLightsBuffer = ctx.get<grl::Rc<urhi::Buffer>>("point_lights_buffer");
        const auto sceneColorTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_depth_texture");
        const auto passDataBuffer = ctx.get<grl::Rc<urhi::Buffer>>("pass_data_buffer");

        auto& sceneBuffers = *ctx.get<GpuSceneBuffers*>("scene_buffers");
        auto& frameResources = *ctx.get<MeshletFrameResources*>("frame_resources");
        auto& materialRegistry = *ctx.get<GpuMaterialRegistry*>("material_registry");

        auto vertexBuffer = sceneBuffers.vertexBuffer();
        auto transformBuffer = sceneBuffers.transformBuffer();
        auto indexBuffer = frameResources.indexScratchBuffer;
        auto drawCmdsBuffer = frameResources.drawCmdBuffer;
        auto drawCountsBuffer = frameResources.drawCountBuffer;

        urhi::ColorAttachment colorAttachment{};
        colorAttachment.target = sceneColorTexture;
        colorAttachment.loadOp = urhi::LoadOp::Clear;

        urhi::DepthStencilAttachment depthAttachment{};
        depthAttachment.target = sceneDepthTexture;
        depthAttachment.loadOp = urhi::LoadOp::Clear;

        urhi::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthAttachment;
        auto& pass = cmd->beginRenderPass(renderPassDesc);

        uint32_t drawBufferIndex = 0;
        for(uint32_t i = 0; i < materialRegistry.templates().size(); i++)
        {
            auto pipeline = materialRegistry.templates().at(i)->getOrCreatePipeline(*m_shaderModule, m_pipelineDesc);
            pass.setPipeline(pipeline);


            pass.setBuffer("camera", cameraBuffer);
            pass.setBuffer("pass", passDataBuffer);
            pass.setBuffer("pointLights", pointLightsBuffer);
            pass.setBuffer("modelData", transformBuffer);

            // pass.pushConstants(modelUniforms); push constants needs to have index into global buffer
            // renderable.material->applyBindings(cmd, pass); need to add bindless textures and a material buffer

            pass.setVertexBuffer(0, vertexBuffer);
            pass.setIndexBuffer(indexBuffer, urhi::IndexFormat::UInt32);
            pass.multiDrawIndexedIndirectCount(drawCmdsBuffer, drawCountsBuffer, drawBufferIndex, i);

            auto templInfo = materialRegistry.templateInfos().at(i);
            drawBufferIndex += templInfo.totalPrimitiveCount;
        }

        pass.end();
    }
}
