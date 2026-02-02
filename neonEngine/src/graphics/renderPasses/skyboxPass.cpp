#include "skyboxPass.h"

namespace Neon
{
    SkyboxRenderPass::SkyboxRenderPass()
    {
        m_screenMesh = Mesh();
        const std::vector screenVertices =
        {
            Vertex { {-1, -1, 0 }, { }, { 0, 0} },
            Vertex { { 1, -1, 0 }, { }, { 1, 0} },
            Vertex { { 1,  1, 0 }, { }, { 1, 1} },
            Vertex { {-1,  1, 0 }, { }, { 0, 1} },
        };

        const std::vector<uint32_t> quadIndices =
        {
            0, 1, 2,
            0, 2, 3
        };

        m_screenMesh.setVertices(screenVertices);
        m_screenMesh.setIndices(quadIndices);

        m_screenMesh.apply();
    }

    void SkyboxRenderPass::execute(const Rc<RHI::CommandList> &cmd, RenderContext &ctx)
    {
        if(!ctx.has("skybox_material")
            || !ctx.has("camera_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture"))
            return;

        MaterialShader& material = *ctx.get<MaterialShader*>("skybox_material");
        const auto cameraBuffer = ctx.get<Rc<RHI::Buffer>>("camera_buffer");
        const auto sceneColorTexture = ctx.get<Rc<RHI::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<Rc<RHI::TextureView>>("scene_depth_texture");

        RHI::ColorAttachment colorAttachment{};
        colorAttachment.texture = sceneColorTexture;
        colorAttachment.loadOp = RHI::LoadOp::Load;
        colorAttachment.storeOp = RHI::StoreOp::Store;

        RHI::DepthStencilAttachment depthAttachment{};
        depthAttachment.texture = sceneDepthTexture;
        depthAttachment.depthLoadOp = RHI::LoadOp::Load;
        depthAttachment.depthStoreOp = RHI::StoreOp::DontCare;

        RHI::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthAttachment;
        cmd->beginRenderPass(renderPassDesc);

        cmd->setPipeline(material.getPipeline());

        cmd->setUniformBuffer("CameraUniforms", cameraBuffer);

        material.bindUniforms(cmd);

        cmd->setVertexBuffer(0, m_screenMesh.getVertexBuffer());
        cmd->setIndexBuffer(m_screenMesh.getIndexBuffer(), RHI::IndexFormat::UInt32);
        cmd->drawIndexed(6);

        cmd->endRenderPass();
    }
}
