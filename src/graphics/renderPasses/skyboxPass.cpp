#include "skyboxPass.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "graphics/graphicsSystem.h"

namespace ion
{
    struct ScreenVert
    {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    SkyboxRenderPass::SkyboxRenderPass()
    {
        const std::vector screenVertices =
        {
            ScreenVert { {-1, -1 }, { 0, 0} },
            ScreenVert { { 1, -1 }, { 1, 0} },
            ScreenVert { { 1,  1 }, { 1, 1} },
            ScreenVert { {-1,  1 }, { 0, 1} },
        };

        const std::vector<uint16_t> quadIndices =
        {
            0, 2, 1,
            0, 3, 2
        };

        auto device = Engine::getSystem<GraphicsSystem>()->device();

        dg::BufferData vertexBufferData{screenVertices.data(), screenVertices.size() * sizeof(ScreenVert)};

        dg::BufferDesc screenVertBufferDesc{};
        screenVertBufferDesc.Name      = "Fullscreen quad vertices";
        screenVertBufferDesc.Size      = sizeof(ScreenVert) * screenVertices.size();
        screenVertBufferDesc.Usage     = dg::USAGE_IMMUTABLE;
        screenVertBufferDesc.BindFlags = dg::BIND_VERTEX_BUFFER;
        device->CreateBuffer(screenVertBufferDesc, &vertexBufferData, &m_screenVertexBuffer);


        dg::BufferData indexBufferData{quadIndices.data(), quadIndices.size() * sizeof(uint16_t)};

        dg::BufferDesc screenIndexBufferDesc{};
        screenIndexBufferDesc.Name      = "Fullscreen quad indices";
        screenIndexBufferDesc.Size      = sizeof(uint16_t) * quadIndices.size();
        screenIndexBufferDesc.Usage     = dg::USAGE_IMMUTABLE;
        screenIndexBufferDesc.BindFlags = dg::BIND_INDEX_BUFFER;
        device->CreateBuffer(screenIndexBufferDesc, &indexBufferData, &m_screenIndexBuffer);

        AssetImportPipeline& importPipeline = Engine::assetImportPipeline();
        m_shaderModule = importPipeline.import<ShaderModule>("shaders/genericSkybox.hlsl");
    }

    void SkyboxRenderPass::execute(const dg::Ref<dg::IDeviceContext>& dc, RenderContext &ctx)
    {
        if(!ctx.has("skybox_material")
            || !ctx.has("camera_buffer")
            || !ctx.has("scene_rtv")
            || !ctx.has("scene_dtv")
            || !ctx.has("pass_data_buffer"))
            return;

        const auto material = ctx.get<MaterialInstance*>("skybox_material");
        if(material == nullptr) return;

        const auto cameraBuffer = ctx.get<dg::Ref<dg::IBuffer>>("camera_buffer");
        const auto sceneRtv = ctx.get<dg::Ref<dg::ITextureView>>("scene_rtv");
        const auto sceneDtv = ctx.get<dg::Ref<dg::ITextureView>>("scene_dtv");
        const auto passDataBuffer = ctx.get<dg::Ref<dg::IBuffer>>("pass_data_buffer");


        PassDefinition passDef{};
        passDef.name = "skybox_pass";
        passDef.topology = dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        passDef.rtvFormats = { dg::TEX_FORMAT_RGBA8_UNORM };
        passDef.dtvFormat = dg::TEX_FORMAT_D32_FLOAT;

        passDef.overrides.depth = DepthPreset::ReadOnly;

        dg::ITextureView* pRTVs[] = { sceneRtv };
        dc->SetRenderTargets(1, pRTVs, sceneDtv, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const auto [pso, srb] = material->materialTemplate()->getOrCreatePipeline(*m_shaderModule, passDef);
        dc->SetPipelineState(pso);

        srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "camera")->Set(cameraBuffer);
        srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "pass")->Set(passDataBuffer);

        dc->CommitShaderResources(srb, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        dc->SetVertexBuffers(0, 1, &m_screenVertexBuffer, nullptr, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        dc->SetIndexBuffer(m_screenIndexBuffer, 0, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        dc->DrawIndexed({ 6, dg::VT_INT16, dg::DRAW_FLAG_NONE });
    }
}
