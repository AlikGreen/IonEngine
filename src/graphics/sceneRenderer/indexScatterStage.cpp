#include "indexScatterStage.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"

namespace ion
{
    IndexScatterStage::IndexScatterStage(const grl::Rc<urhi::Device> &device)
        : m_device(device)
    {
        auto& importPipeline = Engine::assetImportPipeline();

        urhi::ComputePipelineDesc desc{};
        auto module = importPipeline.load<urhi::slang::Module>("shaders/buildIndexBuffer.slang");
        const auto shader = urhi::slang::Compiler::linkToShaderSet({{*module}}).stages()[0];
        desc.shader = m_device->createShader(shader);
        m_pipeline = m_device->createPipeline(desc);
    }

    void IndexScatterStage::execute(urhi::ComputePass &pass, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame)
    {
        pass.setPipeline(m_pipeline);

        pass.setBuffer("gMeshlets", sceneBuffers.meshletBuffer());
        pass.setBuffer("gVisibleMeshlets", frame.visibleMeshlets);
        pass.setBuffer("gCurrentOffsetPerPrimitive", frame.indexCountOrOffsetPerPrimitive);
        pass.setBuffer("gGlobalIndices", sceneBuffers.indexBuffer());
        pass.setBuffer("gScratchIndices", frame.indexScratchBuffer);
        pass.setBuffer("gPrimitives", sceneBuffers.primitiveBuffer());

        pass.dispatchIndirect(frame.indirectDispatchArgs);
    }
}
