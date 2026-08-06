#include "meshletCullStage.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"

namespace ion
{
    struct CameraData1
    {
        glm::vec4 frustumPlanes[6];
        glm::vec3 position;
        uint32_t padding;
    };

    MeshletCullStage::MeshletCullStage(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame)
        : m_device(device)
    {
        auto& importPipeline = Engine::assetImportPipeline();

        dg::BufferDesc bufferDesc;
        bufferDesc.Size      = sizeof(CameraData1);
        bufferDesc.Usage     = dg::USAGE_DYNAMIC;
        bufferDesc.BindFlags = dg::BIND_UNIFORM_BUFFER;
        bufferDesc.CPUAccessFlags = dg::CPU_ACCESS_WRITE;
        device->CreateBuffer( bufferDesc, nullptr, &m_cameraBuffer );

        const auto& graphics = *Engine::getSystem<GraphicsSystem>();

        const auto module = importPipeline.load<ShaderModule>("shaders/meshletCull.hlsl");
        const auto bundle = graphics.shaderRegistry().getOrCreate(*module);

        m_pso = graphics.pipelineRegistry().getOrCreateCompute(bundle);
        m_pso->CreateShaderResourceBinding(&m_srb);

        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gMeshletInstances")->Set(sceneBuffers.meshletInstanceBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gMeshlets")->Set(sceneBuffers.meshletBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gPrimitives")->Set(sceneBuffers.primitiveBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gModelData")->Set(sceneBuffers.transformBuffer()->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gVisibleMeshlets")->Set(frame.visibleMeshlets->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gIndirectDispatchArgs")->Set(frame.indirectDispatchArgs->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gIndexCountPerPrimitive")->Set(frame.indexCountOrOffsetPerPrimitive->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));

        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gDebugLines")->Set(frame.debugLinesBuffer->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gDebugLinesDraw")->Set(frame.linesDrawCmdBuffer->GetDefaultView(dg::BUFFER_VIEW_UNORDERED_ACCESS));

        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gCamera")->Set(m_cameraBuffer->GetDefaultView(dg::BUFFER_VIEW_SHADER_RESOURCE));
    }

    void MeshletCullStage::execute(const dg::Ref<dg::IDeviceContext> &ctx, const glm::mat4 &camTransform, const Camera &camera, const GpuSceneBuffers &sceneBuffers)
    {
        const glm::mat4 flip = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));
        const glm::mat4 invViewMat = camTransform * flip;
        const glm::mat4 viewMat = glm::inverse(invViewMat);

        CameraData1 data{};
        for(size_t i = 0; i < 6; i++)
            data.frustumPlanes[i] = camera.getFrustum(viewMat).planes().at(i);

        data.position = glm::vec3(camTransform[3]);

        ctx->UpdateBuffer(m_cameraBuffer, 0, sizeof(CameraData1), &data, dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        ctx->SetPipelineState(m_pso);

        const uint32_t meshletInstanceCount = sceneBuffers.meshletInstanceCount();
        m_srb->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "pc")->SetInlineConstants(&meshletInstanceCount, 0, 1);

        ctx->DispatchCompute({(meshletInstanceCount  + 63) / 64, 1, 1});
    }
}
