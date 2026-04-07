#pragma once
#include "audioClip.h"
#include "asset/assetImporter.h"

namespace ion
{
class AudioClipImporter final : public AssetImporter<AudioClip>
{
public:
    grl::Box<AudioClip> import(const std::filesystem::path& filepath, const NoOptions&) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
};
}
