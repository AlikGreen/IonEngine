#include "opaqueForwardScenePass.h"

#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

namespace Neon
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

    ForwardSceneRenderPass::ForwardSceneRenderPass(const Rc<RHI::Device>& device)
    {
        m_modelUniformBuffer = device->createUniformBuffer();

        const auto cl = device->createCommandList();
        cl->begin();
        cl->reserveBuffer(m_modelUniformBuffer, sizeof(ModelUniforms));
        device->submit(cl);
    }

    void ForwardSceneRenderPass::execute(const Rc<RHI::CommandList>& cmd, RenderContext &ctx)
    {
        if(!ctx.has("camera_buffer")
            || !ctx.has("renderables")
            || !ctx.has("point_lights_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture"))
        {
            return;
        }

        const auto cameraBuffer = ctx.get<Rc<RHI::Buffer>>("camera_buffer");
        const auto pointLightsBuffer = ctx.get<Rc<RHI::Buffer>>("point_lights_buffer");
        const auto sceneColorTexture = ctx.get<Rc<RHI::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<Rc<RHI::TextureView>>("scene_depth_texture");

        RHI::ColorAttachment colorAttachment{};
        colorAttachment.texture = sceneColorTexture;

        RHI::DepthStencilAttachment depthAttachment{};
        depthAttachment.texture = sceneDepthTexture;

        RHI::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthAttachment;
        cmd->beginRenderPass(renderPassDesc);

        auto& renderables = *ctx.get<std::vector<Renderable>*>("renderables");

        for (const auto& renderable: renderables)
        {
            drawRenderable(cmd, renderable, cameraBuffer, pointLightsBuffer);
        }

        cmd->endRenderPass();
    }


    void ForwardSceneRenderPass::drawRenderable(const Rc<RHI::CommandList> &cmd, const Renderable& renderable, const Rc<RHI::Buffer> &cameraBuffer, const Rc<RHI::Buffer> &pointLightsBuffer) const
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
        cmd->setIndexBuffer(renderable.mesh->getIndexBuffer(), RHI::IndexFormat::UInt32);
        cmd->drawIndexed(primitive.indexCount, 1, primitive.indexStart);
    }

}
