#include "scriptContext.h"

#include <utility>
#include <ranges>

#include "scriptComponent.h"
#include "bindings/ecsScriptBindings.h"
#include "bindings/inputScriptBindings.h"
#include "bindings/loggingScriptBindings.h"
#include "Coral/HostInstance.hpp"
#include "core/engine.h"
#include "core/sceneManager.h"

namespace ion
{
    void ScriptContext::addAssembly(const std::string &name, const AssetRef<ScriptAssemblyData>& assemblyData)
    {
        m_assets[name] = assemblyData;
    }

    void ScriptContext::removeAssembly(const std::string &name)
    {
        m_assets.erase(name);
    }

    Coral::ManagedAssembly * ScriptContext::getLoaded(const std::string &name)
    {
        const auto it = m_assemblies.find(name);
        if(it == m_assemblies.end())
            return nullptr;

        return it->second;
    }

    const Coral::Type* ScriptContext::findType(const std::string &name)
    {
        for(const auto& assembly : m_assemblies | std::ranges::views::values)
        {
            for(const auto& type : assembly->GetLocalTypes())
            {
                 if(type.GetFullName() == name)
                     return &type;
            }
        }

        return nullptr;
    }

    void ScriptContext::reload()
    {
        for(const auto& cb : m_onBeforeUnload)
        {
            cb(*this);
        }

        auto& registry = Engine::sceneManager().getCurrentScene().registry();

        if(m_loaded)
        {
            for(const auto& [entity, comp] : registry.view<ScriptComponent>())
            {
                comp.saveState();
            }

            m_hostInstance->UnloadAssemblyLoadContext(m_loadContext);
        }

        m_loaded = true;

        m_loadContext = m_hostInstance->CreateAssemblyLoadContext(m_name);

        m_componentScriptTypes.clear();
        m_systemScriptTypes.clear();
        m_assemblies.clear();
        for(const auto& [name, asset] : m_assets)
        {
            auto& assembly = m_loadContext.LoadAssemblyFromMemory(asset->data().data(), asset->data().size());
            m_assemblies[name] = &assembly;

            CallBinder callBinder(assembly);

            for(const auto& bindings : m_bindings[name])
            {
                bindings->registerCalls(callBinder);
            }

            assembly.UploadInternalCalls();
        }

        m_componentScriptBaseType = findType("IonEngine.ComponentScript");
        m_systemScriptBaseType    = findType("IonEngine.SystemScript");

        for(const auto& [name, assembly] : m_assemblies)
        {
            for(const auto& type : assembly->GetLocalTypes())
            {
                if(type.IsSubclassOf(*m_componentScriptBaseType))
                    m_componentScriptTypes.push_back(&type);
                if(type.IsSubclassOf(*m_systemScriptBaseType))
                    m_systemScriptTypes.push_back(&type);
            }
        }

        clogr::info("{} component script classes found", m_componentScriptTypes.size());
        clogr::info("{} system script classes found", m_systemScriptTypes.size());

        for(const auto& [entity, comp] : registry.view<ScriptComponent>())
        {
            comp.reload(*this);
        }

        for(const auto& cb : m_onAfterReload)
        {
            cb(*this);
        }
    }


    ScriptContext::ScriptContext(Coral::HostInstance &hostInstance, const AssetRef<ScriptAssemblyData>& ionEngineAssemblyData, std::string name)
        : m_name(std::move(name)), m_hostInstance(&hostInstance)
    {
        addAssembly("IonEngine", ionEngineAssemblyData);
        registerBindings<EcsScriptBindings>("IonEngine");
        registerBindings<InputScriptBindings>("IonEngine");
        registerBindings<LoggingScriptBindings>("IonEngine");
    }
}
