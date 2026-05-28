#pragma once
#include "graphics/renderPass.h"

namespace ion
{
class OcclusionCullingPass final : public RenderPass
{
public:
    void execute(const grl::Rc<urhi::CommandList> &cmd, RenderContext &ctx) override;
};
}
