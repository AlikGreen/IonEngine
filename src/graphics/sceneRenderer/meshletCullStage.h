#pragma once

#include "gpuSceneBuffers.h"
#include "meshletFrameResources.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

namespace ion
{
class MeshletCullStage
{
public:
    MeshletCullStage(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame);
    void execute(const dg::Ref<dg::IDeviceContext> &ctx, const glm::mat4 &camTransform, const Camera &camera, const GpuSceneBuffers& sceneBuffers);
private:
    dg::Ref<dg::IRenderDevice> m_device;
    dg::Ref<dg::IPipelineState> m_pso;
    dg::Ref<dg::IShaderResourceBinding> m_srb;
    dg::Ref<dg::IBuffer> m_cameraBuffer;
};
}
