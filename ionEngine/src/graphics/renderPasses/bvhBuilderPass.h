#pragma once
#include "graphics/renderPass.h"

namespace ion
{
class BvhBuilderPass final : public RenderPass
{
public:
    explicit BvhBuilderPass(const grl::Rc<urhi::Device> &device);
    void execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx) override;
private:
    grl::Rc<urhi::Pipeline> m_triBoundsPipeline;
    grl::Rc<urhi::Buffer> m_triBoundsStorageBuffer;
    grl::Rc<urhi::Buffer> m_triBoundsUniformBuffer;
    grl::Rc<urhi::Buffer> m_triMortonCodes;
    grl::Rc<urhi::Buffer> m_triIndices;

    grl::Rc<urhi::Pipeline> m_bvhPipeline;
    grl::Rc<urhi::Pipeline> m_bvhComputeAABBsPipeline;
    grl::Rc<urhi::Buffer> m_bvhNodesBuffer;
    grl::Rc<urhi::Buffer> m_bvhConstants;

    grl::Rc<urhi::Pipeline> m_meshBoundsPass1Pipeline;
    grl::Rc<urhi::Pipeline> m_meshBoundsPass2Pipeline;

    grl::Rc<urhi::Buffer> m_meshBoundsConstantsBuffer;
    grl::Rc<urhi::Buffer> m_meshGroupBoundsBuffer;
    uint32_t m_meshGroupBoundsBufferSize = 128;

    grl::Rc<urhi::Pipeline> m_radixSortPass1Pipeline;
    grl::Rc<urhi::Pipeline> m_radixSortPass2Pipeline;
    grl::Rc<urhi::Pipeline> m_radixSortPass3Pipeline;

    grl::Rc<urhi::Buffer> m_outputMortonCodes;
    grl::Rc<urhi::Buffer> m_outputIndices;
    grl::Rc<urhi::Buffer> m_radixSortConstants;
    grl::Rc<urhi::Buffer> m_radixHistogramBuffer;
};
}
