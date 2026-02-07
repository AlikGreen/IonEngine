#pragma once

namespace ion
{
class AssetImporter
{
public:
    virtual ~AssetImporter() = default;

    virtual bool isFileSupported(const std::string &filepath) { return true; }
    virtual void* load(const std::string &filepath) = 0;
};
}
