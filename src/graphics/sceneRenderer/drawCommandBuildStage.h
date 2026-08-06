#pragma once
#include "gpuSceneBuffers.h"
#include "meshletFrameResources.h"

namespace ion
{
class DrawCommandBuildStage
{
public:
    DrawCommandBuildStage(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame);
    void execute(const dg::Ref<dg::IDeviceContext>& ctx, const GpuSceneBuffers &sceneBuffers, const GpuMaterialRegistry& matRegistry);
private:
    dg::Ref<dg::IRenderDevice> m_device;
    dg::Ref<dg::IBuffer> m_drawWriteCursorPerTemplate;
    dg::Ref<dg::IPipelineState> m_pso;
    dg::Ref<dg::IShaderResourceBinding> m_srb;
};
}
