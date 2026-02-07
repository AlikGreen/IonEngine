#pragma once
#include "buffer.h"

namespace ion
{
class RenderUtil
{
public:
    static void init();
    static grl::Rc<urhi::Buffer> getQuad();
};
}
