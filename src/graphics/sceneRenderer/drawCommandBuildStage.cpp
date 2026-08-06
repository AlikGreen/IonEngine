#include "drawCommandBuildStage.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"

namespace ion
{
    constexpr uint32_t kMaxMaterialTemplates = 512; // maybe should have global values

    DrawCommandBuildStage::DrawCommandBuildStage(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame)
        : m_device(device)
    {
        auto& importPipeline = Engine::assetImportPipeline();
        const auto& graphics = *Engine::getSystem<GraphicsSystem>();

        dg::BufferDesc bufferDesc;
        bufferDesc.Size      = sizeof(uint32_t)*kMaxMaterialTemplates;
        bufferDesc.Usage     = dg::USAGE_DEFAULT;
        bufferDesc.BindFlags = dg::BIND_UNORDERED_ACCESS | dg::BIND_SHADER_RESOURCE;
        bufferDesc.Mode      = dg::BUFFER_MODE_STRUCTURED;
        bufferDesc.ElementByteStride = sizeof(uint32_t);
        device->CreateBuffer( bufferDesc, nullptr, &m_drawWriteCursorPerTemplate);

        const auto module = importPipeline.load<ShaderModule>("shaders/materialDraws.hlsl");
        const auto bundle = graphics.shaderRegistry().getOrCreate(*module);

        m_pso = graphics.pipelineRegistry().getOrCreateCompute(bundle);
        m_pso->CreateShaderResourceBinding(&m_srb);

        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gIndexCountPerPrimitive")->Set(frame.indexCountOrOffsetPerPrimitive->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gDrawOffsetPerTemplate")->Set(m_drawWriteCursorPerTemplate->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gDrawCountPerTemplate")->Set(frame.drawCountBuffer->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gPrimitives")->Set(sceneBuffers.primitiveBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gDrawCommands")->Set(frame.drawCmdBuffer->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gGlobalIndexAllocationCounter")->Set(frame.globalIndexAllocationCounter->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
    }

    void DrawCommandBuildStage::execute(const dg::Ref<dg::IDeviceContext>& ctx, const GpuSceneBuffers &sceneBuffers, const GpuMaterialRegistry& matRegistry)
    {
        ctx->CopyBuffer(
            matRegistry.templateBaseOffsetsBuffer(),
            0,
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
            m_drawWriteCursorPerTemplate,
            0,
            sizeof(uint32_t)*kMaxMaterialTemplates,
            dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        ctx->SetPipelineState(m_pso);
        ctx->CommitShaderResources(m_srb, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const uint32_t primCount = sceneBuffers.primitiveCount();
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "pc")->SetInlineConstants(&primCount, 0, 1); // 1 Uint

        ctx->DispatchCompute({(primCount + 63) / 64, 1, 1});
    }
}
