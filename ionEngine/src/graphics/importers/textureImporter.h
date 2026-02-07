#pragma once

#include "asset/assetImporter.h"

namespace ion
{
class TextureImporter final : public AssetImporter
{
public:
    void* load(const std::string &filepath) override;
};
}
