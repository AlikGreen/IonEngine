#pragma once
#include "indirectMeshletPipeline.h"
#include "core/system.h"
#include "core/components/transformComponent.h"
#include "graphics/renderer.h"
#include "graphics/components/meshRenderer.h"

namespace ion
{
class RendererSystem final : public System
{
public:
    void startup() override;

    void queueView(const grl::Rc<Renderer>& renderer, const grl::Rc<RenderContext>& ctx, Camera camera, glm::mat4 cameraTransform);
    void update(Scene& scene) override;
    void render(Scene& scene) override;
private:
    struct MaterialInfo
    {
        uint32_t templateIndex;
        uint32_t materialIndex;
        uint32_t primitiveCount;
    };

    struct MaterialTemplateInfo
    {
        uint32_t materialCount;
        uint32_t index;
    };

    struct QueuedView
    {
        grl::Rc<Renderer> renderer;
        grl::Rc<RenderContext> ctx;
        Camera camera;
        glm::mat4 camTransform;
    };

    bool needsUnload(Mesh& mesh, Transform& transform);
    bool needsLoad(entis::Entity entity);

    void unloadMesh(Mesh& mesh, const grl::Rc<urhi::CommandList> &cmd);
    void loadMesh(entis::Entity entity, const MeshRenderer& meshRenderer, const grl::Rc<urhi::CommandList> &cmd);

    std::vector<QueuedView> m_queuedViews;

    std::unordered_set<entis::Entity> m_loadedEntities;

    grl::Rc<urhi::Device> m_device;
    grl::Box<IndirectMeshletPipeline> m_pipeline;
    grl::Box<GpuSceneBuffers> m_sceneBuffers;
    grl::Box<GpuMaterialRegistry> m_matRegistry;
};
}
