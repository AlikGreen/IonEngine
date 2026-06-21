#pragma once
#include <urhi/urhi.h>

#include "gpuSceneBuffers.h"
#include "meshletFrameResources.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

namespace ion
{
class MeshletCullStage
{
public:
    MeshletCullStage(const grl::Rc<urhi::Device> &device);
    void execute(const grl::Rc<urhi::CommandList> &cmd, urhi::ComputePass& pass, const glm::mat4 &camTransform, const Camera &camera, const GpuSceneBuffers& sceneBuffers, const MeshletFrameResources& frame);
private:
    grl::Rc<urhi::Device> m_device;
    grl::Rc<urhi::Pipeline> m_pipeline;
    grl::Rc<urhi::Buffer> m_cameraBuffer;
};
}
