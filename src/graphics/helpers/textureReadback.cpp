#include "textureReadback.h"

#include "gfx.h"

namespace ion
{
    TextureReadback TextureReadback::Create(dg::IRenderDevice *device, dg::IDeviceContext *context, const TextureReadbackDesc &desc)
    {
        TextureReadback readback;
        readback.m_context = context;

        readback.m_width  = desc.width;
        readback.m_height = desc.height;
        readback.m_depth = desc.depth;

        readback.m_x = desc.x;
        readback.m_y = desc.y;
        readback.m_z = desc.z;

        auto srcDesc = desc.srcTexture->GetDesc();
        readback.m_format = srcDesc.Format;

        dg::TextureDesc stagingDesc = srcDesc;
        stagingDesc.Name = "Readback Staging Texture";
        stagingDesc.Usage = dg::USAGE_STAGING;
        stagingDesc.CPUAccessFlags = dg::CPU_ACCESS_READ;
        stagingDesc.BindFlags = dg::BIND_NONE;
        stagingDesc.Width = desc.width;
        stagingDesc.Height = desc.height;
        stagingDesc.Depth = desc.depth;


        device->CreateTexture(stagingDesc, nullptr, &readback.m_stagingTex);

        dg::FenceDesc fenceDesc;
        fenceDesc.Name = "Readback Fence";
        fenceDesc.Type = dg::FENCE_TYPE_GENERAL;
        device->CreateFence(fenceDesc, &readback.m_fence);


        dg::CopyTextureAttribs copyAttribs;
        copyAttribs.pSrcTexture = desc.srcTexture;
        copyAttribs.pDstTexture = readback.m_stagingTex;
        copyAttribs.SrcTextureTransitionMode = dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copyAttribs.DstTextureTransitionMode = dg::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        context->CopyTexture(copyAttribs);

        context->EnqueueSignal(readback.m_fence, 1);

        return readback;
    }

    const std::vector<uint8_t> & TextureReadback::getData()
    {
        if (!hasCopiedData)
        {
            wait();

            dg::Box mapRegion{};
            mapRegion.MinX = m_x;
            mapRegion.MinY = m_y;
            mapRegion.MinZ = m_z;

            mapRegion.MaxX = m_x + m_width;
            mapRegion.MaxY = m_y + m_height;
            mapRegion.MaxZ = m_z + m_depth;

            dg::MappedTextureSubresource mapped;
            m_context->MapTextureSubresource(m_stagingTex, 0, 0, dg::MAP_READ, dg::MAP_FLAG_NONE, &mapRegion, mapped);
            isMapped = true;

            const size_t totalSize = gfx::getTextureSize(m_format, m_width, m_height, m_depth);
            data.resize(totalSize);

            const size_t rowBytes = totalSize / (m_height * m_depth);

            for (uint32_t row = 0; row < m_height; ++row)
            {
                memcpy(data.data() + row * rowBytes,
                       static_cast<const uint8_t*>(mapped.pData) + row * mapped.Stride,
                       rowBytes);
            }

            m_context->UnmapTextureSubresource(m_stagingTex, 0, 0);
            isMapped = false;
            hasCopiedData = true;
        }
        return data;
    }
}
