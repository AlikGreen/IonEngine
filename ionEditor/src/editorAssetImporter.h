#pragma once
#include "asset/AssetId.h"
#include "asset/assetRef.h"
#include "asset/assetStream.h"

namespace ion::Editor
{
class EditorAssetImporter
{
public:
    static AssetId import(const std::string& sourcePath, const std::string& destFolder);

    template<typename T>
    static void saveAsAssetFile(AssetRef<T> asset, const std::string& outputPath)
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

        assetManager.serialize(assetStream, *asset);


        const uint32_t endOfFilePos = assetStream.getCursorPos();
        assetStream.setCursorPos(lengthFieldPos);
        assetStream.write<uint32_t>(endOfFilePos-dataPos);
        assetStream.setCursorPos(endOfFilePos); // restore cursor

        std::vector<uint8_t> data = assetStream.getBuffer();
        grl::File::write(outputPath, data);
    }

    template<typename T>
    static grl::Box<T> readAssetFile(const std::string& path)
    {
        AssetManager& assetManager = Engine::getAssetManager();

        const auto file = grl::File::read(path);
        if(!file.has_value())
            return nullptr;

        const std::vector<uint8_t> data(file->data(), file->data() + file->length());
        auto assetStream = AssetStream(data);

        assetStream.skip(4); // skip the magic for now
        assetStream.skip<uint32_t>(); // skip version for now

        uint32_t assetDataPos = 0;
        assetStream.read<uint32_t>(assetDataPos); // byte offset of asset data
        assetStream.setCursorPos(assetDataPos);

        // skip all of these for now
        // const uint32_t lengthFieldPos = assetStream.getCursorPos();
        // assetStream.write<uint32_t>(0); // byte length of asset data
        // assetStream.write<uint32_t>(0); // timestamp (no use for now)
        // assetStream.write<uint32_t>(0); // flags (none for now)
        // assetStream.write<uint64_t>(asset.id().handle()); // asset id

        // assetStream.write<uint32_t>(dataPos); // edit offset

        return assetManager.deserialize<T>(assetStream);
    }
};
}
