#pragma once
#include "drawCommandBuildStage.h"
#include "gpuSceneBuffers.h"
#include "indexScatterStage.h"
#include "meshletCullStage.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"

namespace ion
{
class IndirectMeshletPipeline
{
public:
    explicit IndirectMeshletPipeline(const grl::Rc<urhi::Device> &device);

    void render(const grl::Rc<urhi::CommandList>& cmd,
                glm::mat4 camTransform,
                const Camera &camera,
                const GpuSceneBuffers& scene,
                const GpuMaterialRegistry& materials);

    MeshletFrameResources& frameResources() { return m_frame; }
private:
    void clearFrameResources(const grl::Rc<urhi::CommandList>& cmd);

    MeshletCullStage m_cullStage;
    DrawCommandBuildStage m_drawCommandBuildStage;
    IndexScatterStage m_indexScatterStage;

    MeshletFrameResources m_frame;
};
}
