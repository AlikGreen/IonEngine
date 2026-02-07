#include "textureImporter.h"

#include <stb_image.h>
#include <filesystem>
#include <algorithm>

#include <clogr.h>
#include "graphics/assets/textureData.h"

namespace ion
{
    urhi::PixelType getPixelTypeFromExtension(const std::string &extension)
    {
        if (extension == "exr" || extension == "hdr")
            return urhi::PixelType::Float;

        return urhi::PixelType::UnsignedByte;
    }

    void* TextureImporter::load(const std::string &filepath)
    {
        std::string ext = std::filesystem::path(filepath).extension().string();
        std::ranges::transform(ext, ext.begin(), tolower);

        if (!ext.empty() && ext[0] == '.')
            ext = ext.substr(1);

        int w, h, channels;
        void *pixels{};

        stbi_set_flip_vertically_on_load(true);

        if (ext == "exr" || ext == "hdr")
            pixels = stbi_loadf(filepath.c_str(), &w, &h, &channels, 4);
        else
            pixels = stbi_load(filepath.c_str(), &w, &h, &channels, 4);

        clogr::ensure(pixels != nullptr, "Failed to load image {}\n", stbi_failure_reason());

        auto* texData = new TextureData();
        texData->width = w;
        texData->height = h;
        texData->pixelLayout = urhi::PixelLayout::RGBA;
        texData->pixelType = getPixelTypeFromExtension(ext);
        texData->data = pixels;
        return texData;
    }
}
