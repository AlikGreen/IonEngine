#include "imageSerializer.h"

#include "asset/assetStream.h"

namespace ion
{
    void ImageSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps &deps, const Image &image)
    {
        assetStream.write(image.width());
        assetStream.write(image.height());

        assetStream.write<uint16_t>(image.format());

        assetStream.write(image.pixels());
    }

    grl::Rc<Image> ImageSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        uint32_t width{};
        assetStream.read(width);
        uint32_t height{};
        assetStream.read(height);
        uint16_t formatInt{};
        assetStream.read<uint16_t>(formatInt);

        std::vector<uint8_t> pixels{};
        assetStream.read(pixels);

        TextureData data{};
        data.width = width;
        data.height = height;
        data.pixelFormat = static_cast<dg::TEXTURE_FORMAT>(formatInt);
        data.data = std::move(pixels);

        return grl::makeRc<Image>(data);
    }
}
