#pragma once
#include <urhi/urhi.h>

#include "gpuSceneBuffers.h"
#include "meshletFrameResources.h"

namespace ion
{
class IndexScatterStage
{
public:
    explicit IndexScatterStage(const grl::Rc<urhi::Device> &device);
    void execute(urhi::ComputePass& pass, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame);
private:
    grl::Rc<urhi::Device> m_device;

    grl::Rc<urhi::Pipeline> m_pipeline;
};
}
