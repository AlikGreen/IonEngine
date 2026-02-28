#include "raytracePass.h"

#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

namespace ion
{
    struct ModelUniforms
    {
        glm::mat4 modelMat{};
        glm::mat4 normalMat{};
        int startIndex{};
        int indexCount{};
    };

    struct MaterialUniforms
    {
        float roughness;
        float metalness;
        alignas(16) glm::vec4 albedo;
        int useAlbedoTexture;
    };

    RayTracePass::RayTracePass(const grl::Rc<urhi::Device>& device)
    {
        m_modelUniformBuffer = device->createUniformBuffer();

        const auto cl = device->createCommandList();
        cl->begin();
        cl->reserveBuffer(m_modelUniformBuffer, sizeof(ModelUniforms));
        device->submit(cl);

        const auto shader = Engine::getAssetManager().import<grl::Rc<urhi::Shader>>("shaders/raytracing/raytrace.slang");

        urhi::ComputePipelineDesc pipelineDesc{};
        pipelineDesc.shader = *shader;

        m_rayTracePipeline = device->createPipeline(pipelineDesc);
    }

    void RayTracePass::execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx)
    {
        if(!ctx.has("camera_buffer")
            || !ctx.has("all_renderables")
            || !ctx.has("point_lights_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture")
            || !ctx.has("bvh_buffer")
            || !ctx.has("sorted_tri_indices_buffer"))
        {
            return;
        }

        const auto cameraBuffer = ctx.get<grl::Rc<urhi::Buffer>>("camera_buffer");
        auto& renderables = *ctx.get<std::vector<Renderable>*>("all_renderables");
        const auto pointLightsBuffer = ctx.get<grl::Rc<urhi::Buffer>>("point_lights_buffer");
        const auto sceneColorTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_depth_texture");

        const auto bvhBuffer = ctx.get<grl::Rc<urhi::Buffer>>("bvh_buffer");

        if(sceneColorTexture->getWidth() <= 0 || sceneColorTexture->getHeight() <= 0)
            return;

        cmd->setPipeline(m_rayTracePipeline);

        cmd->setImage("outputImage", sceneColorTexture, urhi::ImageAccess::WriteOnly);

        for (const auto& renderable: renderables)
        {
            const Primitive primitive = renderable.mesh->getPrimitives().at(renderable.submeshIndex);
            ModelUniforms modelUniforms = {
                renderable.worldMatrix,
                glm::transpose(glm::inverse(glm::mat3(renderable.worldMatrix))),
                static_cast<int>(primitive.indexStart),
                static_cast<int>(primitive.indexCount)
            };
            cmd->updateBuffer(m_modelUniformBuffer, modelUniforms);

            cmd->setUniformBuffer("CameraUniforms", cameraBuffer);
            cmd->setUniformBuffer("ModelUniforms", m_modelUniformBuffer);


            // might need to have a uniform buffer setting the vertex offset/index start
            cmd->setStorageBuffer("bvh", bvhBuffer);
            cmd->setStorageBuffer("vertexBuffer", renderable.mesh->getVertexBuffer());
            cmd->setStorageBuffer("indices", renderable.mesh->getIndexBuffer());
            cmd->dispatch({(sceneColorTexture->getWidth()+15)/16, (sceneDepthTexture->getHeight()+15)/16, 1});
        }
    }
}
