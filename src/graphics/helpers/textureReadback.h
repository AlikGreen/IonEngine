#pragma once

#include "GraphicsAccessories.hpp"

namespace ion
{
struct TextureReadbackDesc
{
    dg::Ref<dg::ITexture> srcTexture;

    uint32_t width = 1;
    uint32_t height = 1;
    uint32_t depth = 1;

    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t z = 0;
};

class TextureReadback
{
public:
    static TextureReadback Create(
        dg::IRenderDevice* device,
        dg::IDeviceContext* context,
        const TextureReadbackDesc &desc);

    [[nodiscard]] bool isReady() const { return m_fence->GetCompletedValue() >= 1; }
    void wait() const { m_fence->Wait(1); }

    const std::vector<uint8_t>& getData();
private:
    dg::Ref<dg::ITexture>       m_stagingTex;
    dg::Ref<dg::IDeviceContext> m_context;
    dg::Ref<dg::IFence>         m_fence;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_depth = 0;

    uint32_t m_x = 0;
    uint32_t m_y = 0;
    uint32_t m_z = 0;
    dg::TEXTURE_FORMAT m_format{};

    std::vector<uint8_t>    data;
    bool                    hasCopiedData = false;
    bool                    isMapped = false;
};
}
