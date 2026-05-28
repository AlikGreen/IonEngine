#include "skyboxPass.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"

namespace ion
{
    SkyboxRenderPass::SkyboxRenderPass()
    {
        m_screenMesh = Mesh();
        const std::vector screenVertices =
        {
            Vertex { {-1, -1, 1 }, { }, { 0, 0} },
            Vertex { { 1, -1, 1 }, { }, { 1, 0} },
            Vertex { { 1,  1, 1 }, { }, { 1, 1} },
            Vertex { {-1,  1, 1 }, { }, { 0, 1} },
        };

        const std::vector<uint32_t> quadIndices =
        {
            0, 2, 1,
            0, 3, 2
        };

        m_screenMesh.vertices(screenVertices);
        m_screenMesh.indices(quadIndices);


        urhi::DepthState depthState{};
        depthState.hasDepthTarget   = true;
        depthState.enableDepthTest  = true;
        depthState.enableDepthWrite = false;
        depthState.compareOp = urhi::CompareOp::LessOrEqual;

        urhi::RasterizerState rasterState{};
        rasterState.cullMode = urhi::CullMode::Back;

        urhi::BlendState blendState{};
        blendState.enableBlend    = false;

        urhi::ColorAttachmentDesc colorAttach{};
        colorAttach.blend  = blendState;
        colorAttach.format = urhi::PixelFormat::RGBA8UNorm;

        m_pipelineDesc.depthState       = depthState;
        m_pipelineDesc.rasterizerState  = rasterState;
        m_pipelineDesc.colorAttachments = { colorAttach };

        m_pipelineDesc.depthAttachmentFormat = urhi::PixelFormat::Depth32Float;

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        m_shaderModule = importPipeline.import<urhi::slang::Module>("shaders/genericSkybox.slang");
    }

    void SkyboxRenderPass::execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx)
    {
        if(!ctx.has("skybox_material")
            || !ctx.has("camera_buffer")
            || !ctx.has("scene_color_texture")
            || !ctx.has("scene_depth_texture")
            || !ctx.has("pass_data_buffer"))
            return;

        const auto material = ctx.get<MaterialInstance*>("skybox_material");
        if(material == nullptr) return;

        const auto cameraBuffer = ctx.get<grl::Rc<urhi::Buffer>>("camera_buffer");
        const auto sceneColorTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_color_texture");
        const auto sceneDepthTexture = ctx.get<grl::Rc<urhi::TextureView>>("scene_depth_texture");
        const auto passDataBuffer = ctx.get<grl::Rc<urhi::Buffer>>("pass_data_buffer");

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
        auto& pass = cmd->beginRenderPass(renderPassDesc);

        const auto pipeline = material->materialTemplate()->getOrCreatePipeline(*m_shaderModule, m_pipelineDesc);
        pass.setPipeline(pipeline);

        material->applyBindings(cmd, pass);
        pass.setBuffer("camera", cameraBuffer);
        pass.setBuffer("pass", passDataBuffer);

        pass.setVertexBuffer(0, m_screenMesh.vertexBuffer());
        pass.setIndexBuffer(m_screenMesh.indexBuffer(), urhi::IndexFormat::UInt32);
        pass.drawIndexed(6);

        pass.end();
    }
}
