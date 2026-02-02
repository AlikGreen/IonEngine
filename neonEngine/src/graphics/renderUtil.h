#pragma once
#include "buffer.h"

namespace Neon
{
class RenderUtil
{
public:
    static void init();
    static Rc<RHI::Buffer> getQuad();
};
}
