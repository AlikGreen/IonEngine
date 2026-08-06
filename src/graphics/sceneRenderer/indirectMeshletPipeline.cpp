#include "indirectMeshletPipeline.h"

#include "graphics/components/camera.h"
#include "graphics/helpers/gfx.h"

namespace ion
{
    IndirectMeshletPipeline::IndirectMeshletPipeline(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &scene, const GpuMaterialRegistry &materials)
        : m_frame(device), m_cullStage(device, scene, m_frame), m_drawCommandBuildStage(device, scene, m_frame),
            m_indexScatterStage(device, scene, m_frame)
    {
    }

    void IndirectMeshletPipeline::render(const dg::Ref<dg::IDeviceContext> &cmd, glm::mat4 camTransform, const Camera &camera, const GpuSceneBuffers &scene, const GpuMaterialRegistry &materials)
    {
        clearFrameResources(cmd);

        m_cullStage.execute(cmd, camTransform, camera, scene);
        m_drawCommandBuildStage.execute(cmd, scene, materials);
        m_indexScatterStage.execute(cmd, m_frame);
    }

    void IndirectMeshletPipeline::clearFrameResources(const dg::Ref<dg::IDeviceContext> &cmd)
    {
        gfx::fillBuffer(cmd, m_frame.visibleMeshlets, 0);
        gfx::fillBuffer(cmd, m_frame.visibleMeshlets, 0u);
        gfx::fillBuffer(cmd, m_frame.indirectDispatchArgs, 0u);
        gfx::fillBuffer(cmd, m_frame.indexCountOrOffsetPerPrimitive, 0u);
        gfx::fillBuffer(cmd, m_frame.globalIndexAllocationCounter, 0u);
        gfx::fillBuffer(cmd, m_frame.indexScratchBuffer, 0u);
        gfx::fillBuffer(cmd, m_frame.drawCmdBuffer, 0u);
        gfx::fillBuffer(cmd, m_frame.drawCountBuffer, 0u);
        gfx::fillBuffer(cmd, m_frame.linesDrawCmdBuffer, 0u);
    }
}
