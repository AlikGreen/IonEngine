#pragma once

#include "gpuSceneBuffers.h"
#include "meshletFrameResources.h"

namespace ion
{
class IndexScatterStage
{
public:
    explicit IndexScatterStage(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame);
    void execute(const dg::Ref<dg::IDeviceContext>& ctx, const MeshletFrameResources &frame);
private:
    dg::Ref<dg::IRenderDevice> m_device{};
    dg::Ref<dg::IPipelineState> m_pso{};
    dg::Ref<dg::IShaderResourceBinding> m_srb{};
};
}
