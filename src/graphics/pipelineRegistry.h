#pragma once
#include "shaders/shaderBundle.h"

namespace ion
{
enum class BlendPreset { Opaque, AlphaBlend, Additive, Premultiplied };
enum class DepthPreset { ReadWrite, ReadOnly, Disabled };

using BlendOverride = std::variant<BlendPreset, dg::BlendStateDesc>;
using DepthOverride = std::variant<DepthPreset, dg::DepthStencilStateDesc>;

struct PipelineOverrides
{
    std::optional<BlendOverride> blend{};
    std::optional<DepthOverride> depth{};

    std::optional<dg::CULL_MODE> cullMode{};
    std::optional<dg::FILL_MODE> fillMode{};
    std::optional<bool> scissorEnabled{};
};

struct PassDefinition
{
    std::string name{};
    std::vector<dg::TEXTURE_FORMAT> rtvFormats{};
    dg::TEXTURE_FORMAT dtvFormat = dg::TEX_FORMAT_UNKNOWN;
    dg::PRIMITIVE_TOPOLOGY topology = dg::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    PipelineOverrides overrides{};
};

class PipelineRegistry
{
public:
    explicit PipelineRegistry(dg::Ref<dg::IRenderDevice> device);
    dg::Ref<dg::IPipelineState> getOrCreateGraphics(const ShaderBundle& shader, const PassDefinition &definition);
    dg::Ref<dg::IPipelineState> getOrCreateCompute(const ShaderBundle &shader);
private:
    dg::Ref<dg::IRenderDevice> m_device;

    std::unordered_map<uint32_t, dg::Ref<dg::IPipelineState>> m_psoCache{};

    static std::vector<dg::ShaderResourceVariableDesc> createResourceLayout(const std::vector<ShaderResource> &resources);
    static dg::BlendStateDesc createBlendState(const BlendOverride &blendOverride, size_t rtvCount);
    static dg::DepthStencilStateDesc createDepthState(const DepthOverride &depthOverride);

    size_t pipelineHash(const dg::GraphicsPipelineStateCreateInfo &ci);
};
}
