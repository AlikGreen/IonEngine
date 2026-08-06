#include "indexScatterStage.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"

namespace ion
{
    IndexScatterStage::IndexScatterStage(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame)
        : m_device(device)
    {
        auto& importPipeline = Engine::assetImportPipeline();
        const auto& graphics = *Engine::getSystem<GraphicsSystem>();

        const auto module = importPipeline.load<ShaderModule>("shaders/buildIndexBuffer.hlsl");
        const auto bundle = graphics.shaderRegistry().getOrCreate(*module);

        m_pso = graphics.pipelineRegistry().getOrCreateCompute(bundle);
        m_pso->CreateShaderResourceBinding(&m_srb);

        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gMeshlets")->Set(sceneBuffers.meshletBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gVisibleMeshlets")->Set(frame.visibleMeshlets->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gCurrentOffsetPerPrimitive")->Set(frame.indexCountOrOffsetPerPrimitive->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gGlobalIndices")->Set(sceneBuffers.indexBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gScratchIndices")->Set(frame.indexScratchBuffer->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gPrimitives")->Set(sceneBuffers.primitiveBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
    }

    void IndexScatterStage::execute(const dg::Ref<dg::IDeviceContext>& ctx, const MeshletFrameResources &frame)
    {
        ctx->SetPipelineState(m_pso);
        ctx->DispatchComputeIndirect({ frame.indirectDispatchArgs, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION });
    }
}
