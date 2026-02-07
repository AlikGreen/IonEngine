#pragma once

#include <utility>

#include "assetManager.h"
#include "core/engine.h"

namespace ion
{
template <typename T>
class AssetRef
{
public:
    AssetRef() : assetManager(nullptr), cachedAsset(nullptr), m_id(0) {  };
    AssetRef(T* asset) : assetManager(nullptr), cachedAsset(asset)      {  }

    T* operator->() const
    {
        updateCachedAsset();
        return cachedAsset;
    }

    T& operator*() const
    {
        updateCachedAsset();
        return *cachedAsset;
    }

    T* get() const
    {
        updateCachedAsset();
        return cachedAsset;
    }

    AssetId id() const
    {
        return m_id;
    }

    bool operator==(const AssetRef other) const
    {
        return m_id == other.m_id;
    }

    bool operator!=(const AssetRef other) const
    {
        return m_id != other.m_id;
    }

    operator bool() const
    {
        return m_id.isValid();
    }
private:
    friend class AssetManager;

    AssetRef(AssetManager* assetManager, const AssetId m_id) : assetManager(assetManager), cachedAsset(nullptr), m_id(m_id) {  };

    void updateCachedAsset() const
    {
        if (cachedAsset == nullptr && assetManager != nullptr && assetManager->isValid(m_id))
        {
            cachedAsset = &assetManager->getAssetRef<T>(m_id);
        }
    }

    AssetManager* assetManager;
    mutable T* cachedAsset = nullptr;
    AssetId m_id{};
};

template<typename T>
AssetRef<T> AssetManager::addAsset(T* asset, std::string name)
{
    const auto typeIndex = std::type_index(typeid(T));

    if(name.empty())
    {
        name = typeIndex.name();
    }

    AssetId handle = generateID();
    assets.emplace(handle, asset);
    assetsMetadata.emplace(handle, AssetMetadata { name, typeIndex, "" });

    assetHandles.push_back(handle);
    return AssetRef<T>(this, handle);
}

template<typename T>
AssetRef<T> AssetManager::addAsset(T asset, std::string name)
{
    const auto typeIndex = std::type_index(typeid(T));

    if(name.empty())
    {
        name = typeIndex.name();
    }

    AssetId handle = generateID();
    T* heapAsset = new T(std::move(asset));  // Allocate on heap
    assets.emplace(handle, heapAsset);
    assetsMetadata.emplace(handle, AssetMetadata { name, typeIndex, "" });

    assetHandles.push_back(handle);
    return AssetRef<T>(this, handle);
}

template<typename T>
AssetRef<T> AssetManager::import(const std::string& filepath)
{
    const std::filesystem::path fullPath = getFullPath(filepath);

    clogr::ensure(exists(fullPath), "File was not found\n{}", filepath);

    const auto typeIndex = std::type_index(typeid(T));

    clogr::ensure(importers.contains(typeIndex), "Cannot load object of type {}", typeid(T).name());

    const std::string name = fullPath.stem().string();
    const auto assetLoader = importers.at(typeIndex).get();
    T* asset = static_cast<T*>(assetLoader->load(fullPath.string()));

    AssetId handle = generateID();
    assets.emplace(handle, asset);
    assetsMetadata.emplace(handle, AssetMetadata { name, typeIndex, filepath });

    assetHandles.push_back(handle);
    return AssetRef<T>(this, handle);
}

template<typename T>
AssetRef<T> AssetManager::getAsset(const AssetId assetHandle)
{
    clogr::ensure(isValid(assetHandle), "Tried to get an asset that did not exist.");
    return AssetRef<T>(this, assetHandle);
}

template<typename T>
void AssetManager::serialize(AssetStream& assetStream, T& asset)
{
    clogr::ensure(serializers.contains(typeid(T)), "Tried to serialize that did not have a serializer registered for.");

    const grl::Box<AssetSerializer>& serializer = serializers.at(typeid(T));
    serializer->serialize(assetStream, *this, &asset);
}

template<typename T>
grl::Box<T> AssetManager::deserialize(AssetStream& assetStream)
{
    clogr::ensure(serializers.contains(typeid(T)), "Tried to deserialize that did not have a serializer registered for.");

    const grl::Box<AssetSerializer>& serializer = serializers.at(typeid(T));
    T* asset = static_cast<T*>(serializer->deserialize(assetStream, *this));
    return grl::Box<T>(asset);
}
}
