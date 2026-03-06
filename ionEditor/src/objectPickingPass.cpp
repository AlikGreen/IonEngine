#include "objectPickingPass.h"

#include <tiny_gltf.h>

namespace ion::Editor
{
    struct ModelUniforms
    {
        glm::mat4 model{};
        uint32_t entityId = 0;
    };

    ObjectPickingPass::ObjectPickingPass(const grl::Rc<urhi::Device> &device)
    {
        AssetManager& assetManager = Engine::getAssetManager();
        const auto shaders = assetManager.import<std::vector<urhi::ShaderEntryPoint>>("shaders/objectPicker.slang");

        MaterialDescription desc{};
        desc.name = "Editor viewport object picker";
        desc.shaders = shaders;
        desc.cullMode = urhi::CullMode::Back;
        desc.blendEnabled = false;
        desc.depthTest = false;
        desc.depthWrite = false;

        m_pickingMaterial = MaterialShader(desc);

        m_modelUniformBuffer = device->createBuffer({ urhi::BufferUsage::Uniform, sizeof(ModelUniforms) });
    }

    void ObjectPickingPass::execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx)
    {
        if(!ctx.has("all_renderables") ||
            !ctx.has("picker_color_texture") ||
            !ctx.has("picker_depth_texture") ||
            !ctx.has("camera_buffer"))
            return;

        const auto& renderables = *ctx.get<std::vector<Renderable>*>("all_renderables");
        const auto pickerColorTexture = ctx.get<grl::Rc<urhi::TextureView>>("picker_color_texture");
        const auto pickerDepthTexture = ctx.get<grl::Rc<urhi::TextureView>>("picker_depth_texture");
        const auto& cameraBuffer = ctx.get<grl::Rc<urhi::Buffer>>("camera_buffer");

        urhi::ColorAttachment colorAttachment{};
        colorAttachment.target = pickerColorTexture;
        colorAttachment.clearValue = urhi::ClearColorFloat{ 0.0f, 0.0f, 0.0f, 1.0f };

        urhi::DepthStencilAttachment depthStencilAttachment{};
        depthStencilAttachment.target = pickerDepthTexture;
        depthStencilAttachment.clearDepth = 1.0f;

        urhi::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        renderPassDesc.depthAttachment = depthStencilAttachment;
        auto pass = cmd->beginRenderPass(renderPassDesc);

        pass->setPipeline(m_pickingMaterial.getPipeline());
        pass->setUniformBuffer("CameraUniforms", cameraBuffer);

        for(auto renderable : renderables)
        {
            ModelUniforms modelUniforms = { renderable.worldMatrix, static_cast<uint32_t>(renderable.entity.id()) };
            cmd->updateBuffer(m_modelUniformBuffer, modelUniforms);
            pass->setUniformBuffer("ModelUniforms", m_modelUniformBuffer);

            const Primitive primitive = renderable.mesh->getPrimitives().at(renderable.submeshIndex);

            pass->setVertexBuffer(0, renderable.mesh->getVertexBuffer());
            pass->setIndexBuffer(renderable.mesh->getIndexBuffer(), urhi::IndexFormat::UInt32);
            pass->drawIndexed(primitive.indexCount, 1, primitive.indexStart);
        }

        pass->end();
    }
}
