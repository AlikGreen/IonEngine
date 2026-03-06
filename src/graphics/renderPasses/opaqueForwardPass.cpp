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
        colorAttachment.target = sceneColorTexture;
        colorAttachment.loadOp = urhi::LoadOp::Clear;

        urhi::DepthStencilAttachment depthAttachment{};
        depthAttachment.target = sceneDepthTexture;

        urhi::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthAttachment;
        const auto pass = cmd->beginRenderPass(renderPassDesc);

        auto& renderables = *ctx.get<std::vector<Renderable>*>("opaque_renderables");

        for (const auto& renderable: renderables)
        {
            ModelUniforms modelUniforms = { renderable.worldMatrix };

            pass->setPipeline(renderable.material->getPipeline());

            pass->setUniformBuffer("camera", cameraBuffer);
            pass->setUniformBuffer("pointLights", pointLightsBuffer);
            pass->pushConstants(modelUniforms);

            renderable.material->bindUniforms(cmd, pass);

            const Primitive primitive = renderable.mesh->getPrimitives().at(renderable.submeshIndex);

            pass->setVertexBuffer(0, renderable.mesh->getVertexBuffer());
            pass->setIndexBuffer(renderable.mesh->getIndexBuffer(), urhi::IndexFormat::UInt32);
            pass->drawIndexed(primitive.indexCount, 1, primitive.indexStart);
        }

        pass->end();
    }
}
