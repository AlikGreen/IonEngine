#pragma once
#include "shaderReflector.h"

namespace ion
{
class ShaderBundle
{
public:
    std::vector<ShaderResource> resources;
    std::vector<dg::LayoutElement> layoutElements;

    dg::Ref<dg::IShader> vs{};
    dg::Ref<dg::IShader> ps{};
    dg::Ref<dg::IShader> cs{};
};
}
