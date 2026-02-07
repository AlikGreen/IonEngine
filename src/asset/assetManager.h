#pragma once

#include <filesystem>
#include <typeindex>

#include "AssetId.h"
#include "assetImporter.h"
#include "assetSerializer.h"
#include <clogr.h>

namespace ion
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
    AssetRef<T> getAsset(AssetId assetHandle);

    template<typename T>
    bool assetIsType(const AssetId assetID) const
    {
        return assetsMetadata.at(assetID).type == typeid(T);
    }

    template<typename T>
    grl::Box<T> loadUnmanaged(const std::string& filepath)
    {
        const std::filesystem::path fullPath = getFullPath(filepath);

        clogr::ensure(exists(fullPath), "File was not found\n{}", filepath);
        clogr::ensure(importers.contains(typeid(T)), "Cannot load object of type {}", typeid(T).name());

        const auto assetLoader = importers.at(typeid(T)).get();
        T* asset = static_cast<T*>(assetLoader->load(fullPath.string()));

        return grl::Box<T>(asset);
    }

    template<typename SerializerType, typename AssetType, typename... Args> requires std::derived_from<SerializerType, AssetSerializer>
    SerializerType& registerSerializer(Args&&... args)
    requires std::constructible_from<SerializerType, Args...>
    {
        auto* serializer = new SerializerType(std::forward<Args>(args)...);
        serializers.emplace(typeid(AssetType), grl::Box<SerializerType>(serializer));
        return *serializer;
    }

    template<typename LoaderType, typename AssetType, typename... Args> requires std::derived_from<LoaderType, AssetImporter>
    LoaderType& registerImporter(Args&&... args)
    requires std::constructible_from<LoaderType, Args...>
    {
        grl::Rc<LoaderType> loader = grl::makeBox<LoaderType>(std::forward<Args>(args)...);
        importers.emplace(typeid(AssetType), loader);
        return *loader;
    }

    template<typename T>
    void serialize(AssetStream& assetStream, T& asset);
    template<typename T>
    grl::Box<T> deserialize(AssetStream& assetStream);

    std::vector<AssetId> getAllAssetIDs();
    AssetMetadata getMetadata(AssetId handle);
    AssetId generateID();
    bool isValid(AssetId id) const;

    static std::string getFullPath(const std::string& filePath);
private:
    template <typename T>
    friend class AssetRef;

    template<typename T>
    T& getAssetRef(const AssetId id)
    {
        clogr::ensure(id.isValid(), "Invalid AssetID");
        return *static_cast<T*>(assets.at(id));
    }

    uint64_t nextHandle = 1;

    std::unordered_map<std::type_index, grl::Box<AssetSerializer>> serializers;
    std::unordered_map<std::type_index, grl::Rc<AssetImporter>> importers;

    std::vector<AssetId> assetHandles;
    std::unordered_map<AssetId, void*> assets;
    std::unordered_map<AssetId, AssetMetadata> assetsMetadata;
};
}