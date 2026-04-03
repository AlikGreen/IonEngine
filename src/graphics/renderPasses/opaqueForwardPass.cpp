#include "opaqueForwardPass.h"

#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

namespace ion
{
    struct ModelUniforms
    {
        glm::mat4 model{};
        glm::mat4 normalMatrix{};
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
            glm::mat4 normalMatrix = glm::transpose(glm::inverse(renderable.worldMatrix));
            ModelUniforms modelUniforms = { renderable.worldMatrix, normalMatrix };

            pass->setPipeline(renderable.material->materialTemplate()->pipeline());

            pass->setUniformBuffer("camera", cameraBuffer);
            pass->setUniformBuffer("pointLights", pointLightsBuffer);
            pass->pushConstants(modelUniforms);

            renderable.material->bind(cmd, pass);

            const Primitive primitive = renderable.mesh->primitives().at(renderable.submeshIndex);

            pass->setVertexBuffer(0, renderable.mesh->vertexBuffer());
            pass->setIndexBuffer(renderable.mesh->indexBuffer(), urhi::IndexFormat::UInt32);
            pass->drawIndexed(primitive.indexCount, 1, primitive.indexStart);
        }

        pass->end();
    }
}