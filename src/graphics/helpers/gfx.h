#pragma once
#include "graphics/graphicsSystem.h"

namespace ion::gfx
{
    void init(const GraphicsSystem& graphicsSystem);

    void fillBuffer(const dg::Ref<dg::IDeviceContext> &ctx, const dg::Ref<dg::IBuffer> &buffer, uint32_t value, uint32_t offset = 0, uint32_t size = 0);
    size_t getTextureSize(dg::TEXTURE_FORMAT format, uint32_t width, uint32_t height, uint32_t depth);


    template<typename T>
    requires std::is_trivially_copyable_v<T>
    void updateDynamicBuffer(const dg::Ref<dg::IDeviceContext> &ctx, const dg::Ref<dg::IBuffer> &buffer, const T& data)
    {
        void* mappedData = nullptr;
        ctx->MapBuffer(buffer, dg::MAP_WRITE, dg::MAP_FLAG_DISCARD, mappedData);
        memcpy(mappedData, &data, sizeof(T));
        ctx->UnmapBuffer(buffer, dg::MAP_WRITE);
    }
}
