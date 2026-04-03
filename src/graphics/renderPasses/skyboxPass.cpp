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
            0, 2, 1,
            0, 3, 2
        };

        m_screenMesh.vertices(screenVertices);
        m_screenMesh.indices(quadIndices);
    }

    void SkyboxRenderPass::execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx)
    {
        if(!ctx.has("skybox_material")
            || !ctx.has("camera_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture"))
            return;

        MaterialInstance& material = *ctx.get<MaterialInstance*>("skybox_material");
        const auto cameraBuffer = ctx.get<grl::Rc<urhi::Buffer>>("camera_buffer");
        const auto sceneColorTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_depth_texture");

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
        const auto pass = cmd->beginRenderPass(renderPassDesc);

        pass->setPipeline(material.materialTemplate()->pipeline());

        pass->setUniformBuffer("camera", cameraBuffer);
        material.bind(cmd, pass);

        pass->setVertexBuffer(0, m_screenMesh.vertexBuffer());
        pass->setIndexBuffer(m_screenMesh.indexBuffer(), urhi::IndexFormat::UInt32);
        pass->drawIndexed(6);

        pass->end();
    }
}
