#pragma once
#include "scriptAssemblyData.h"
#include "scriptBindings.h"
#include "asset/assetRef.h"

#include <unordered_map>

namespace entis
{
    class Registry;
}

namespace ion
{
class ScriptContext
{
public:
    ScriptContext() = default;

    ScriptContext(const ScriptContext&) = delete;
    ScriptContext& operator=(const ScriptContext&) = delete;

    ScriptContext(ScriptContext&&) noexcept = default;
    ScriptContext& operator=(ScriptContext&&) noexcept = default;

    void addAssembly(const std::string& name, const AssetRef<ScriptAssemblyData>& assemblyData);
    void removeAssembly(const std::string& name);

    Coral::ManagedAssembly* getLoaded(const std::string& name);
    const Coral::Type* findType(const std::string& name);

    template<typename T, typename... Args>
    requires std::is_base_of_v<ScriptBindings, T> && std::is_constructible_v<T, Args...>
    void registerBindings(const std::string& assemblyName, Args&&... args)
    {
        m_bindings[assemblyName].emplace_back(grl::makeBox<T>(std::forward<Args>(args)...));
    }

    void reload();
    bool isLoaded() const { return m_loaded; }

    void onBeforeUnload(const std::function<void(ScriptContext&)> &callback) { m_onBeforeUnload.push_back(callback); }
    void onAfterReload(const std::function<void(ScriptContext&)> &callback) { m_onAfterReload.push_back(callback); }

    const std::vector<const Coral::Type*>& componentScriptTypes() { return m_componentScriptTypes; }
    const std::vector<const Coral::Type*>& systemScriptTypes() { return m_systemScriptTypes; }
private:
    friend class ScriptManager;
    ScriptContext(Coral::HostInstance& hostInstance, const AssetRef<ScriptAssemblyData>& ionEngineAssemblyData, std::string name);

    std::unordered_map<std::string, AssetRef<ScriptAssemblyData>> m_assets;
    std::unordered_map<std::string, Coral::ManagedAssembly*> m_assemblies;
    std::unordered_map<std::string, std::vector<grl::Box<ScriptBindings>>> m_bindings;

    std::vector<std::function<void(ScriptContext&)>> m_onBeforeUnload;
    std::vector<std::function<void(ScriptContext&)>> m_onAfterReload;


    std::vector<const Coral::Type*> m_componentScriptTypes;
    std::vector<const Coral::Type*> m_systemScriptTypes;

    std::string m_name;
    bool m_loaded = false;
    Coral::AssemblyLoadContext m_loadContext{};
    Coral::HostInstance* m_hostInstance{};
    AssetRef<ScriptAssemblyData> m_ionEngineAssemblyData;

    const Coral::Type* m_componentScriptBaseType{};
    const Coral::Type* m_systemScriptBaseType{};
};
}
