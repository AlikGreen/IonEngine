#include "imageImporter.h"

#include <stb_image.h>

namespace ion
{
    grl::Box<Image> ImageImporter::import(const std::filesystem::path &filepath, const NoOptions&)
    {
        const std::string ext = std::filesystem::path(filepath).extension().string();

        int w, h, channels;
        void *pixels{};

        stbi_set_flip_vertically_on_load(true);

        const bool hdr = ext == ".exr" || ext == ".hdr";

        if (hdr)
            pixels = stbi_loadf(filepath.string().c_str(), &w, &h, &channels, 4);
        else
            pixels = stbi_load(filepath.string().c_str(), &w, &h, &channels, 4);

        clogr::ensure(pixels != nullptr, "Failed to load image {}\n", stbi_failure_reason());

        const size_t size = w * h * 4 * (hdr ? 4 : 1);
        const std::vector buffer(static_cast<uint8_t *>(pixels), static_cast<uint8_t *>(pixels) + size);

        TextureData texData{};
        texData.width = w;
        texData.height = h;
        texData.data = buffer;
        texData.pixelFormat = hdr ? dg::TEX_FORMAT_RGBA32_FLOAT : dg::TEX_FORMAT_RGBA8_UNORM;

        return grl::makeBox<Image>(texData);
    }

    bool ImageImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".png" || src.extension() == ".hdr" || src.extension() == ".jpg" || src.extension() == ".jpeg";
    }
}
