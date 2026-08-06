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
private:
    struct ImporterBase
    {
        virtual ~ImporterBase() = default;
        [[nodiscard]] virtual bool canImport(const std::filesystem::path& src) const = 0;
    };

    template<typename T>
    struct DefaultImporterWrapper : ImporterBase
    {
        virtual grl::Box<T> importDefault(const std::filesystem::path& src) = 0;
    };

    template<typename T, typename Opt>
    struct TypedImporterWrapper final : DefaultImporterWrapper<T>
    {
        grl::Box<AssetImporter<T, Opt>> inner;

        grl::Box<T> importDefault(const std::filesystem::path& src) override
        {
            if constexpr (std::is_default_constructible_v<Opt>)
            {
                return inner->import(src, Opt{});
            } else
            {
                return nullptr;
            }
        }

        [[nodiscard]] bool canImport(const std::filesystem::path& src) const override
        {
            return inner->canImport(src);
        }
    };

public:
    AssetImportPipeline(AssetRegistry& assetRegistry, ResourceFS& rfs)
        : m_assetRegistry(assetRegistry), m_rfs(rfs) { }

    AssetImportPipeline(const AssetImportPipeline&) = delete;
    AssetImportPipeline& operator=(const AssetImportPipeline&) = delete;

    template<typename Importer, typename... Args>
    requires std::is_constructible_v<Importer, Args...> &&
             std::is_base_of_v<AssetImporter<typename Importer::AssetType, typename Importer::OptionsType>, Importer>
    Importer& registerImporter(Args&&... args)
    {
        using T = typename Importer::AssetType;
        using Opt = typename Importer::OptionsType;

        auto wrapper = grl::makeRc<TypedImporterWrapper<T, Opt>>();
        wrapper->inner = grl::makeBox<Importer>(std::forward<Args>(args)...);
        auto* raw = wrapper->inner.get();
        m_importers[typeid(T)] = std::move(wrapper);

        return *static_cast<Importer*>(raw);
    }

    template<typename T, typename Opt>
    AssetRef<T> import(const std::filesystem::path& srcPath, const Opt& options)
    {
        auto data = load<T, Opt>(srcPath, options);
        if (!data) return nullptr;
        return m_assetRegistry.add<T>(grl::Rc<T>(data.release()));
    }

    template<typename T>
    AssetRef<T> import(const std::filesystem::path& srcPath)
    {
        auto data = load<T>(srcPath);
        if (!data) return nullptr;
        return m_assetRegistry.add<T>(grl::Rc<T>(data.release()));
    }

    template<typename T, typename Opt>
    grl::Box<T> load(const std::filesystem::path& srcPath, const Opt& options)
    {
        namespace fs = std::filesystem;
        if (!m_importers.contains(typeid(T))) return nullptr;

        auto src = m_rfs.resolve(srcPath.string());
        if (src.empty()) src = fs::path(srcPath);
        if (!exists(src)) return nullptr;

        auto* base = m_importers.at(typeid(T)).get();
        if (!base->canImport(src)) return nullptr;

        auto* wrapper = dynamic_cast<TypedImporterWrapper<T, Opt>*>(base);
        if (!wrapper) return nullptr;

        return wrapper->inner->import(src, options);
    }

    template<typename T>
    grl::Box<T> load(const std::filesystem::path& srcPath)
    {
        namespace fs = std::filesystem;
        if (!m_importers.contains(typeid(T))) return nullptr;

        auto src = m_rfs.resolve(srcPath.string());
        if (src.empty()) src = fs::path(srcPath);
        if (!exists(src))
        {
            clogr::error("Could not find file at path: {}", srcPath.string());
            return nullptr;
        }

        auto* base = m_importers.at(typeid(T)).get();
        if (!base->canImport(src)) return nullptr;

        auto* wrapper = dynamic_cast<DefaultImporterWrapper<T>*>(base);
        if (!wrapper) return nullptr;

        return wrapper->importDefault(src);
    }
private:
    AssetRegistry& m_assetRegistry;
    ResourceFS& m_rfs;
    std::unordered_map<std::type_index, grl::Rc<ImporterBase>> m_importers;
};
}
