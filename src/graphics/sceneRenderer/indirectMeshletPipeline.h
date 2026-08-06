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
    explicit IndirectMeshletPipeline(const dg::Ref<dg::IRenderDevice> &device, const GpuSceneBuffers &scene, const GpuMaterialRegistry &materials);

    void render(const dg::Ref<dg::IDeviceContext>& cmd,
                glm::mat4 camTransform,
                const Camera &camera,
                const GpuSceneBuffers& scene,
                const GpuMaterialRegistry& materials);

    MeshletFrameResources& frameResources() { return m_frame; }
private:
    void clearFrameResources(const dg::Ref<dg::IDeviceContext>& cmd);

    MeshletFrameResources m_frame;

    MeshletCullStage m_cullStage;
    DrawCommandBuildStage m_drawCommandBuildStage;
    IndexScatterStage m_indexScatterStage;
};
}
