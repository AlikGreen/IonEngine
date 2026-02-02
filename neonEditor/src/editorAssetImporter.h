#pragma once
#include "asset/assetID.h"
#include "asset/assetRef.h"
#include "asset/assetStream.h"

namespace Neon
{
class EditorAssetImporter
{
public:
    static AssetID import(const std::string& sourcePath, const std::string& destFolder);

    template<typename T>
    static void saveAsAssetFile(AssetRef<T> asset, std::string& outputPath)
    {
        AssetManager& assetManager = Engine::getAssetManager();

        AssetStream assetStream;

        constexpr uint8_t magic[4] =
        {
            'N', 'E', 'O', 'N'
        };

        assetStream.write(magic, 4);
        assetStream.write<uint32_t>(0); // version

        const uint32_t offsetFieldPos = assetStream.getCursorPos();
        assetStream.write<uint32_t>(0); // byte offset of asset data

        const uint32_t lengthFieldPos = assetStream.getCursorPos();
        assetStream.write<uint32_t>(0); // byte length of asset data
        assetStream.write<uint32_t>(0); // timestamp (no use for now)
        assetStream.write<uint32_t>(0); // flags (none for now)
        assetStream.write<uint64_t>(asset.id().handle()); // asset id

        const uint32_t dataPos = assetStream.getCursorPos();
        assetStream.setCursorPos(offsetFieldPos);
        assetStream.write<uint32_t>(dataPos); // edit offset
        assetStream.setCursorPos(dataPos); // restore cursor

        assetManager.serialize(assetStream, asset);

        // save this to a file
        std::vector<uint8_t> data = assetStream.getBuffer();

        const uint32_t endOfFilePos = assetStream.getCursorPos();
        assetStream.setCursorPos(lengthFieldPos);
        assetStream.write<uint32_t>(endOfFilePos-dataPos);
        assetStream.setCursorPos(endOfFilePos); // restore cursor


        File::write(outputPath, assetStream.getBuffer());
    }
};
}
