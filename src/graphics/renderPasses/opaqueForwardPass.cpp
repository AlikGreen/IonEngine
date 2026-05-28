#include "opaqueForwardPass.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

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
            || !ctx.has("opaque_renderables")
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

        urhi::ColorAttachment colorAttachment{};
        colorAttachment.target = sceneColorTexture;
        colorAttachment.loadOp = urhi::LoadOp::Clear;

        urhi::DepthStencilAttachment depthAttachment{};
        depthAttachment.target = sceneDepthTexture;

        urhi::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthAttachment;
        auto& pass = cmd->beginRenderPass(renderPassDesc);

        const auto& renderables = *ctx.get<std::vector<Renderable>*>("opaque_renderables");

        for (const auto& renderable: renderables)
        {
            const glm::mat4 normalMatrix = glm::transpose(glm::inverse(renderable.worldMatrix));
            ModelUniforms modelUniforms = { renderable.worldMatrix, normalMatrix };

            auto pipeline = renderable.material->materialTemplate()->getOrCreatePipeline(*m_shaderModule, m_pipelineDesc);
            pass.setPipeline(pipeline);

            pass.setBuffer("camera", cameraBuffer);
            pass.setBuffer("pass", passDataBuffer);
            pass.setBuffer("pointLights", pointLightsBuffer);
            pass.pushConstants(modelUniforms);

            renderable.material->applyBindings(cmd, pass);

            const Primitive primitive = renderable.mesh->primitives().at(renderable.submeshIndex);

            pass.setVertexBuffer(0, renderable.mesh->vertexBuffer());
            pass.setIndexBuffer(renderable.mesh->indexBuffer(), urhi::IndexFormat::UInt32);
            pass.drawIndexed(primitive.indexCount, 1, primitive.indexStart);
        }

        pass.end();
    }
}