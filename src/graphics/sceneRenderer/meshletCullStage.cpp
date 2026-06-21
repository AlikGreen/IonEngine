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

    MeshletCullStage::MeshletCullStage(const grl::Rc<urhi::Device> &device)
        : m_device(device)
    {
        auto& importPipeline = Engine::assetImportPipeline();

        urhi::ComputePipelineDesc desc{};
        auto module = importPipeline.load<urhi::slang::Module>("shaders/meshletCull.slang");
        const auto shader = urhi::slang::Compiler::linkToShaderSet({{*module}}).stages()[0];
        desc.shader = m_device->createShader(shader);
        m_pipeline = m_device->createPipeline(desc);
        m_cameraBuffer = m_device->createBuffer({ urhi::BufferUsage::Uniform, sizeof(CameraData1) });
    }

    void MeshletCullStage::execute(const grl::Rc<urhi::CommandList> &cmd, urhi::ComputePass& pass, const glm::mat4 &camTransform, const Camera &camera, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame)
    {
        const glm::mat4 flip = glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, -1));
        const glm::mat4 invViewMat = camTransform * flip;
        const glm::mat4 viewMat = glm::inverse(invViewMat);

        CameraData1 data{};
        for(size_t i = 0; i < 6; i++)
            data.frustumPlanes[i] = camera.getFrustum(viewMat).planes().at(i);

        data.position = glm::vec3(camTransform[3]);

        cmd->updateBuffer(m_cameraBuffer, data);

        pass.setPipeline(m_pipeline);

        pass.setBuffer("gMeshletInstances", sceneBuffers.meshletInstanceBuffer());
        pass.setBuffer("gMeshlets", sceneBuffers.meshletBuffer());
        pass.setBuffer("gPrimitives", sceneBuffers.primitiveBuffer());
        pass.setBuffer("gModelData", sceneBuffers.transformBuffer());
        pass.setBuffer("gVisibleMeshlets", frame.visibleMeshlets);
        pass.setBuffer("gIndirectDispatchArgs", frame.indirectDispatchArgs);
        pass.setBuffer("gIndexCountPerPrimitive", frame.indexCountOrOffsetPerPrimitive);

        pass.setBuffer("gDebugLines", frame.debugLinesBuffer);
        pass.setBuffer("gDebugLinesDraw", frame.linesDrawCmdBuffer);

        pass.setBuffer("gCamera", m_cameraBuffer);

        uint32_t meshletInstanceCount = sceneBuffers.meshletInstanceCount();
        pass.pushConstants(meshletInstanceCount);

        pass.dispatch((meshletInstanceCount  + 63) / 64, 1, 1);
    }
}
