#pragma once
#include "asset/assetImporter.h"

namespace ion
{
class AudioClipImporter final : public AssetImporter
{
public:
    void* load(const std::string &filepath) override;
};
}
