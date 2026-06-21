#pragma once
#include "device.h"
#include "gpuSceneBuffers.h"
#include "meshletFrameResources.h"

namespace ion
{
class DrawCommandBuildStage
{
public:
    DrawCommandBuildStage(const grl::Rc<urhi::Device> &device);
    void execute(const grl::Rc<urhi::CommandList> &cmd, urhi::ComputePass& pass, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame, const GpuMaterialRegistry& matRegistry);
private:
    grl::Rc<urhi::Device> m_device;

    grl::Rc<urhi::Buffer> m_drawWriteCursorPerTemplate;

    grl::Rc<urhi::Pipeline> m_pipeline;
};
}
