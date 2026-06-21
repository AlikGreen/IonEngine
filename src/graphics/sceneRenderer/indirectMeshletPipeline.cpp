#include "indirectMeshletPipeline.h"

#include "graphics/components/camera.h"

namespace ion
{
    IndirectMeshletPipeline::IndirectMeshletPipeline(const grl::Rc<urhi::Device> &device)
        : m_cullStage(device), m_drawCommandBuildStage(device),
            m_indexScatterStage(device), m_frame(device)
    {
    }

    void IndirectMeshletPipeline::render(const grl::Rc<urhi::CommandList> &cmd, glm::mat4 camTransform, const Camera &camera, const GpuSceneBuffers &scene, const GpuMaterialRegistry &materials)
    {
        clearFrameResources(cmd);

        auto& computePass = cmd->beginComputePass();

        m_cullStage.execute(cmd, computePass, camTransform, camera, scene, m_frame);
        m_drawCommandBuildStage.execute(cmd, computePass, scene, m_frame, materials);
        m_indexScatterStage.execute(computePass, scene, m_frame);

        computePass.end();
    }

    void IndirectMeshletPipeline::clearFrameResources(const grl::Rc<urhi::CommandList> &cmd)
    {
        cmd->fillBuffer(m_frame.visibleMeshlets, 0u);
        cmd->fillBuffer(m_frame.indirectDispatchArgs, 0u);
        cmd->fillBuffer(m_frame.indexCountOrOffsetPerPrimitive, 0u);
        cmd->fillBuffer(m_frame.globalIndexAllocationCounter, 0u);
        cmd->fillBuffer(m_frame.indexScratchBuffer, 0u);
        cmd->fillBuffer(m_frame.drawCmdBuffer, 0u);
        cmd->fillBuffer(m_frame.drawCountBuffer, 0u);
        cmd->fillBuffer(m_frame.linesDrawCmdBuffer, 0u);
    }
}
