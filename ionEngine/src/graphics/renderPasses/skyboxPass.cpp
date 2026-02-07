#include "skyboxPass.h"

namespace ion
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

    void SkyboxRenderPass::execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx)
    {
        if(!ctx.has("skybox_material")
            || !ctx.has("camera_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture"))
            return;

        MaterialShader& material = *ctx.get<MaterialShader*>("skybox_material");
        const auto cameraBuffer = ctx.get<grl::Rc<urhi::Buffer>>("camera_buffer");
        const auto sceneColorTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_depth_texture");

        urhi::ColorAttachment colorAttachment{};
        colorAttachment.texture = sceneColorTexture;
        colorAttachment.loadOp = urhi::LoadOp::Load;
        colorAttachment.storeOp = urhi::StoreOp::Store;

        urhi::DepthStencilAttachment depthAttachment{};
        depthAttachment.texture = sceneDepthTexture;
        depthAttachment.depthLoadOp = urhi::LoadOp::Load;
        depthAttachment.depthStoreOp = urhi::StoreOp::DontCare;

        urhi::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthAttachment;
        cmd->beginRenderPass(renderPassDesc);

        cmd->setPipeline(material.getPipeline());

        cmd->setUniformBuffer("CameraUniforms", cameraBuffer);

        material.bindUniforms(cmd);

        cmd->setVertexBuffer(0, m_screenMesh.getVertexBuffer());
        cmd->setIndexBuffer(m_screenMesh.getIndexBuffer(), urhi::IndexFormat::UInt32);
        cmd->drawIndexed(6);

        cmd->endRenderPass();
    }
}
