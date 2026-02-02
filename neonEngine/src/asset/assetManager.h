#pragma once

#include <filesystem>
#include <typeindex>

#include "assetID.h"
#include "assetImporter.h"
#include "assetSerializer.h"
#include "debug.h"
#include "log.h"

namespace Neon
{

struct AssetMetadata
{
    std::string name;
    std::type_index type;
    std::filesystem::path physicalPath;
};

template<typename T>
class AssetRef;

class AssetManager
{
public:
    AssetManager() = default;

    AssetManager (const AssetManager&) = delete;
    AssetManager& operator= (const AssetManager&) = delete;

    template<typename T>
    AssetRef<T> addAsset(T* asset, std::string name = "");
    template<typename T>
    AssetRef<T> addAsset(T asset, std::string name = "");
    template<typename T>
    AssetRef<T> import(const std::string& filepath);
    template<typename T>
    AssetRef<T> getAsset(AssetID assetHandle);

    template<typename T>
    bool assetIsType(const AssetID assetID) const
    {
        return assetsMetadata.at(assetID).type == typeid(T);
    }

    template<typename T>
    Box<T> loadUnmanaged(const std::string& filepath)
    {
        const std::filesystem::path fullPath = getFullPath(filepath);

        Debug::ensure(exists(fullPath), "File was not found\n{}", filepath);
        Debug::ensure(importers.contains(typeid(T)), "Cannot load object of type {}", typeid(T).name());

        const auto assetLoader = importers.at(typeid(T)).get();
        T* asset = static_cast<T*>(assetLoader->load(fullPath.string()));

        return Box<T>(asset);
    }

    template<typename SerializerType, typename AssetType, typename... Args> requires std::derived_from<SerializerType, AssetSerializer>
    void registerSerializer(Args&&... args)
    requires std::constructible_from<SerializerType, Args...>
    {
        auto* serializer = new SerializerType(std::forward<Args>(args)...);
        serializers.emplace(typeid(AssetType), Box<SerializerType>(serializer));
    }

    template<typename LoaderType, typename AssetType, typename... Args> requires std::derived_from<LoaderType, AssetImporter>
    void registerImporter(Args&&... args)
    requires std::constructible_from<LoaderType, Args...>
    {
        Rc<LoaderType> loader = makeBox<LoaderType>(std::forward<Args>(args)...);

        importers.emplace(typeid(AssetType), loader);
    }

    template<typename T>
    void serialize(AssetStream& assetStream, AssetRef<T> asset);
    template<typename T>
    AssetRef<T> deserialize(AssetStream& assetStream);

    std::vector<AssetID> getAllAssetIDs();
    AssetMetadata getMetadata(AssetID handle);
    AssetID generateID();
    bool isValid(AssetID id) const;

    static std::string getFullPath(const std::string& filePath);
private:
    template <typename T>
    friend class AssetRef;

    template<typename T>
    T& getAssetRef(const AssetID id)
    {
        Debug::ensure(id.isValid(), "Invalid AssetID");
        return *static_cast<T*>(assets.at(id));
    }

    uint64_t nextHandle = 1;

    std::unordered_map<std::type_index, Box<AssetSerializer>> serializers;
    std::unordered_map<std::type_index, Rc<AssetImporter>> importers;

    std::vector<AssetID> assetHandles;
    std::unordered_map<AssetID, void*> assets;
    std::unordered_map<AssetID, AssetMetadata> assetsMetadata;
};
}