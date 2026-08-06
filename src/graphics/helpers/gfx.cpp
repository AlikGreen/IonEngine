#include "gfx.h"

#include "GraphicsAccessories.hpp"
#include "asset/assetImportPipeline.h"
#include "core/engine.h"
#include "graphics/graphicsSystem.h"
#include "graphics/shaders/shaderModule.h"

namespace ion::gfx
{
    static dg::Ref<dg::IPipelineState> s_fillBufferPSO;
    static dg::Ref<dg::IShaderResourceBinding> s_fillBufferSRB;


    void init(const GraphicsSystem& graphicsSystem)
    {
        auto& importPipeline = Engine::assetImportPipeline();

        const auto shaderModule = importPipeline.load<ShaderModule>("shaders/util/fillBuffer.hlsl");

        const auto bundle = graphicsSystem.shaderRegistry().getOrCreate(*shaderModule);
        s_fillBufferPSO = graphicsSystem.pipelineRegistry().getOrCreateCompute(bundle);

        s_fillBufferPSO->CreateShaderResourceBinding(&s_fillBufferSRB);
    }

    void fillBuffer(const dg::Ref<dg::IDeviceContext> &ctx, const dg::Ref<dg::IBuffer> &buffer, const uint32_t value, const uint32_t offset, const uint32_t size)
    {
        struct FillBufferPC
        {
            uint32_t offset;
            uint32_t size;
            uint32_t value;
        };

        const uint32_t count = size  == 0 ? (buffer->GetDesc().Size - offset) / 4 : size / 4;

        const FillBufferPC pc{ offset, count, value };

        s_fillBufferSRB->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "pc")->SetInlineConstants(&pc, 0, 3);
        s_fillBufferSRB->GetVariableByName(dg::SHADER_TYPE_COMPUTE, "gBuffer")->Set(buffer);

        ctx->SetPipelineState(s_fillBufferPSO);

        ctx->DispatchCompute({ (count + 64) / 64, 1, 1 });
    }


    size_t getTextureSize(dg::TEXTURE_FORMAT format, uint32_t width, uint32_t height, uint32_t depth)
    {
        const auto& fmtAttribs = GetTextureFormatAttribs(format);

        uint32_t bytesPerPixel = fmtAttribs.ComponentSize *
            (fmtAttribs.ComponentType != dg::COMPONENT_TYPE_COMPRESSED
                ? fmtAttribs.NumComponents
                : 1);

        return static_cast<size_t>(width) * height * depth * bytesPerPixel;
    }
}
