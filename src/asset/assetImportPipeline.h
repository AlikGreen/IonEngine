#pragma once
#include "assetFileWriter.h"
#include "AssetId.h"
#include "assetImporter.h"
#include "assetRegistry.h"
#include "assetSerializerRegistry.h"
#include "core/resourceFs.h"

namespace ion
{
class AssetImportPipeline
{
public:
    AssetImportPipeline(AssetRegistry& assetRegistry, ResourceFS& rfs)
        : m_assetRegistry(assetRegistry), m_rfs(rfs) { }

    AssetImportPipeline(const AssetImportPipeline&) = delete;
    AssetImportPipeline& operator=(const AssetImportPipeline&) = delete;

    template<typename T, typename Importer, typename... Args>
    requires std::is_constructible_v<Importer, Args...> && std::is_base_of_v<AssetImporter<T>, Importer>
    Importer& registerImporter(Args&&... args)
    {
        auto wrapper = grl::makeRc<ImporterWrapper<T>>();
        wrapper->inner = grl::makeBox<Importer>(std::forward<Args>(args)...);
        auto* raw = wrapper->inner.get();
        m_importers[typeid(T)] = std::move(wrapper);

        return *dynamic_cast<Importer*>(raw);
    }

    template<typename T>
    AssetRef<T> import(const std::filesystem::path& srcPath, const std::filesystem::path& dstDir)
    {
        namespace fs = std::filesystem;
        if(!m_importers.contains(typeid(T))) return nullptr;

        const auto dir = m_rfs.resolve(dstDir.string());

        auto src = m_rfs.resolve(srcPath.string());
        if(src.empty()) src = fs::path(srcPath);
        if(!exists(src)) return nullptr;

        auto importer = dynamic_cast<ImporterWrapper<T>*>(m_importers.at(typeid(T)).get());
        if(importer == nullptr) return nullptr;

        auto data = importer->inner->import(src);
        if (!data) return nullptr;

        const AssetId id = AssetId::random();
        AssetFileWriter writer(id, m_assetRegistry.stableId<T>());

        m_assetRegistry.serialize(writer.bodyStream(), *data);
        if(writer.bodyStream().size() != 0)
        {
            const auto dstPath = dir / (srcPath.stem().string() + ".ion");
            writer.write(dstPath);
        }

        grl::Rc<T> rc(data.release());
        return m_assetRegistry.add(std::move(rc), id);
    }

    template<typename T>
    grl::Box<T> load(const std::filesystem::path& srcPath)
    {
        namespace fs = std::filesystem;
        if (!m_importers.contains(typeid(T))) return nullptr;

        auto src = m_rfs.resolve(srcPath.string());
        if (src.empty()) src = fs::path(srcPath);
        if (!exists(src)) return nullptr;

        auto* importer = dynamic_cast<ImporterWrapper<T>*>(m_importers.at(typeid(T)).get());
        if (importer == nullptr) return nullptr;

        return importer->inner->import(src);
    }
private:
    struct ImporterBase;

    AssetRegistry& m_assetRegistry;
    ResourceFS& m_rfs;
    std::unordered_map<std::type_index, grl::Rc<ImporterBase>> m_importers;

    struct ImporterBase
    {
        virtual ~ImporterBase() = default;
    };

    template<typename T>
    struct ImporterWrapper final : ImporterBase
    {
        grl::Box<AssetImporter<T>> inner;
    };
};
}
