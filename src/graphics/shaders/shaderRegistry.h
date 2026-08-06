#pragma once
#include "shaderBundle.h"
#include "shaderModule.h"

namespace ion
{
class ShaderRegistry
{
public:
    explicit ShaderRegistry(const dg::Ref<dg::IRenderDevice> &device);
    ShaderBundle getOrCreate(const ShaderModule& module, const ShaderProcessDesc& desc = {});
private:
    static uint32_t createShaderVariantKey(const ShaderProcessDesc& desc, const ShaderModule &module);

    dg::Ref<dg::IRenderDevice> m_device;

    std::unordered_map<uint32_t, ShaderBundle> m_bundleCache;
};
}
