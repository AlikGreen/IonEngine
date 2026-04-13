#pragma once
#include "core/system.h"
#include "scriptAssembly.h"
#include "scriptModule.h"
#include "Coral/HostInstance.hpp"


namespace ion
{
class ScriptingSystem final : public System
{
public:
    void startup() override;
    void update() override;
    void shutdown() override;

    ScriptAssembly loadAssembly(std::filesystem::path path, const std::string& name);
    std::optional<ScriptAssembly> getAssembly(const std::string &name);
    bool removeAssembly(const std::string &name);

    void reload();

    template<typename T, typename... Args>
    requires std::is_base_of_v<ScriptModule, T> && std::is_constructible_v<T, Args...>
    void registerModule(const std::string& assemblyName, Args&&... args)
    {
        auto rawModule = new T(std::forward<Args>(args)...);
        m_assemblyModules[assemblyName].emplace_back(grl::Box<T>(rawModule));

        if(m_assemblies.contains(assemblyName))
        {
            auto assembly = m_assemblies[assemblyName];
            static_cast<ScriptModule*>(rawModule)->registerInternalCalls(assembly);
            assembly.uploadInternalCalls();
            static_cast<ScriptModule*>(rawModule)->onAssemblyLoaded(assembly);
        }
    }
private:
    Coral::AssemblyLoadContext m_loadContext{};
    Coral::HostInstance m_hostInstance;

    std::unordered_map<std::string, ScriptAssembly> m_assemblies;
    std::unordered_map<std::string, std::vector<grl::Box<ScriptModule>>> m_assemblyModules;
};
}
