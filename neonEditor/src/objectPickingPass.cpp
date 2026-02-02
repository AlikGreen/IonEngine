#include "objectPickingPass.h"

#include <tiny_gltf.h>

namespace Neon::Editor
{
    struct ModelUniforms
    {
        glm::mat4 model{};
        uint32_t entityId = 0;
    };

    ObjectPickingPass::ObjectPickingPass(const Rc<RHI::Device> &device)
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shader = assetManager.import<Rc<RHI::Shader>>("shaders/objectPicker.slang");

        MaterialDescription desc{};
        desc.name = "Editor viewport object picker";
        desc.shader = shader;
        desc.cullMode = RHI::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = false;
        desc.depthWrite = false;

        m_pickingMaterial = MaterialShader(desc);

        m_modelUniformBuffer = device->createUniformBuffer();

        auto cmd = device->createCommandList();
        cmd->begin();

        cmd->reserveBuffer(m_modelUniformBuffer, sizeof(ModelUniforms));

        device->submit(cmd);
    }

    void ObjectPickingPass::execute(const Rc<RHI::CommandList>& cmd, RenderContext &ctx)
    {
        if(!ctx.has("renderables") ||
            !ctx.has("picker_color_texture") ||
            !ctx.has("picker_depth_texture") ||
            !ctx.has("camera_buffer"))
            return;

        const auto& renderables = *ctx.get<std::vector<Renderable>*>("renderables");
        const auto pickerColorTexture = ctx.get<Rc<RHI::TextureView>>("picker_color_texture");
        const auto pickerDepthTexture = ctx.get<Rc<RHI::TextureView>>("picker_depth_texture");
        const auto& cameraBuffer = ctx.get<Rc<RHI::Buffer>>("camera_buffer");

        RHI::ColorAttachment colorAttachment{};
        colorAttachment.texture = pickerColorTexture;
        colorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };

        RHI::DepthStencilAttachment depthStencilAttachment{};
        depthStencilAttachment.texture = pickerDepthTexture;

        RHI::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        cmd->beginRenderPass(renderPassDesc);

        cmd->setPipeline(m_pickingMaterial.getPipeline());
        cmd->setUniformBuffer("CameraUniforms", cameraBuffer);

        for(auto renderable : renderables)
        {
            ModelUniforms modelUniforms = { renderable.worldMatrix, (uint32_t)renderable.entity.id() };
            cmd->updateBuffer(m_modelUniformBuffer, modelUniforms);
            cmd->setUniformBuffer("ModelUniforms", m_modelUniformBuffer);

            const Primitive primitive = renderable.mesh->getPrimitives().at(renderable.submeshIndex);

            cmd->setVertexBuffer(0, renderable.mesh->getVertexBuffer());
            cmd->setIndexBuffer(renderable.mesh->getIndexBuffer(), RHI::IndexFormat::UInt32);
            cmd->drawIndexed(primitive.indexCount, 1, primitive.indexStart);
        }

        cmd->endRenderPass();
    }
}
