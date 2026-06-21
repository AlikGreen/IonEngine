#include "drawCommandBuildStage.h"

#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "slang/compiler.h"

namespace ion
{
    DrawCommandBuildStage::DrawCommandBuildStage(const grl::Rc<urhi::Device> &device)
        : m_device(device)
    {
        auto& importPipeline = Engine::assetImportPipeline();

        urhi::ComputePipelineDesc desc{};
        auto module = importPipeline.load<urhi::slang::Module>("shaders/materialDraws.slang");
        const auto shader = urhi::slang::Compiler::linkToShaderSet({{*module}}).stages()[0];
        desc.shader = m_device->createShader(shader);
        m_pipeline = m_device->createPipeline(desc);

        constexpr uint32_t kMaxMaterialTemplates = 512; // maybe should have global values
        m_drawWriteCursorPerTemplate = m_device->createBuffer({ urhi::BufferUsage::Storage, sizeof(uint32_t)*kMaxMaterialTemplates});
    }

    void DrawCommandBuildStage::execute(const grl::Rc<urhi::CommandList> &cmd, urhi::ComputePass& pass, const GpuSceneBuffers &sceneBuffers, const MeshletFrameResources &frame, const GpuMaterialRegistry& matRegistry)
    {
        cmd->copyBuffer(matRegistry.templateBaseOffsetsBuffer(), m_drawWriteCursorPerTemplate);

        pass.setPipeline(m_pipeline);

        pass.setBuffer("gIndexCountPerPrimitive", frame.indexCountOrOffsetPerPrimitive);
        pass.setBuffer("gDrawOffsetPerTemplate", m_drawWriteCursorPerTemplate);
        pass.setBuffer("gDrawCountPerTemplate", frame.drawCountBuffer);
        pass.setBuffer("gPrimitives", sceneBuffers.primitiveBuffer());
        pass.setBuffer("gDrawCommands", frame.drawCmdBuffer);
        pass.setBuffer("gGlobalIndexAllocationCounter", frame.globalIndexAllocationCounter);

        uint32_t primCount = sceneBuffers.primitiveCount();
        pass.pushConstants(primCount);

        pass.dispatch((primCount + 63) / 64, 1, 1);
    }
}
