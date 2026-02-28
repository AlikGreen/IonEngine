#include "opaqueForwardPass.h"

#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

namespace ion
{
    struct ModelUniforms
    {
        glm::mat4 model{};
    };

    struct MaterialUniforms
    {
        float roughness;
        float metalness;
        alignas(16) glm::vec4 albedo;
        int useAlbedoTexture;
    };

    OpaqueForwardPass::OpaqueForwardPass(const grl::Rc<urhi::Device>& device)
    {
        m_modelUniformBuffer = device->createUniformBuffer();

        const auto cl = device->createCommandList();
        cl->begin();
        cl->reserveBuffer(m_modelUniformBuffer, sizeof(ModelUniforms));
        device->submit(cl);
    }

    void OpaqueForwardPass::execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx)
    {
        if(!ctx.has("camera_buffer")
            || !ctx.has("opaque_renderables")
            || !ctx.has("point_lights_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture"))
        {
            return;
        }

        const auto cameraBuffer = ctx.get<grl::Rc<urhi::Buffer>>("camera_buffer");
        const auto pointLightsBuffer = ctx.get<grl::Rc<urhi::Buffer>>("point_lights_buffer");
        const auto sceneColorTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_depth_texture");

        urhi::ColorAttachment colorAttachment{};
        colorAttachment.texture = sceneColorTexture;

        urhi::DepthStencilAttachment depthAttachment{};
        depthAttachment.texture = sceneDepthTexture;

        urhi::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthAttachment;
        cmd->beginRenderPass(renderPassDesc);

        auto& renderables = *ctx.get<std::vector<Renderable>*>("opaque_renderables");

        for (const auto& renderable: renderables)
        {
            drawRenderable(cmd, renderable, cameraBuffer, pointLightsBuffer);
        }

        cmd->endRenderPass();
    }


    void OpaqueForwardPass::drawRenderable(const grl::Rc<urhi::CommandList> &cmd, const Renderable& renderable, const grl::Rc<urhi::Buffer> &cameraBuffer, const grl::Rc<urhi::Buffer> &pointLightsBuffer) const
    {
        ModelUniforms modelUniforms = { renderable.worldMatrix };
        cmd->updateBuffer(m_modelUniformBuffer, modelUniforms);

        cmd->setPipeline(renderable.material->getPipeline());

        cmd->setUniformBuffer("CameraUniforms", cameraBuffer);
        cmd->setUniformBuffer("PointLightUniforms", pointLightsBuffer);
        cmd->setUniformBuffer("ModelUniforms", m_modelUniformBuffer);

        renderable.material->bindUniforms(cmd);

        const Primitive primitive = renderable.mesh->getPrimitives().at(renderable.submeshIndex);

        cmd->setVertexBuffer(0, renderable.mesh->getVertexBuffer());
        cmd->setIndexBuffer(renderable.mesh->getIndexBuffer(), urhi::IndexFormat::UInt32);
        cmd->drawIndexed(primitive.indexCount, 1, primitive.indexStart);
    }

}
