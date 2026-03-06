#pragma once
#include "graphics/renderPass.h"
#include <entis/entis.h>

#include "graphics/components/meshRenderer.h"

namespace ion
{
class OpaqueForwardPass final : public RenderPass
{
public:
    void execute(const grl::Rc<urhi::CommandList>& cmd, RenderContext &ctx) override;
};
}
