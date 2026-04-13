#include "scriptingSystem.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <entis/entis.h>

#include "core/engine.h"
#include "core/sceneManager.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/components/meshRenderer.h"
#include "timers/scopeTimer.h"
#include "Coral/HostInstance.hpp"
#include "core/resourceFS.h"
#include "modules/ecsScriptModule.h"
#include "modules/inputScriptModule.h"
#include "modules/loggingScriptModule.h"

namespace ion
{
    void cSharpExceptionCallback(std::string_view errorMessage)
    {
        clogr::error("C# Exception: {}", errorMessage);
    }

    void ScriptingSystem::startup()
    {
        auto& registry = Engine::sceneManager().getCurrentScene().registry().asTypeErased();
        registry.registerType<Transform>();
        registry.registerType<Camera>();
        registry.registerType<Tag>();
        registry.registerType<MeshRenderer>();

        clogr::info("Tag size {}", sizeof(Tag));

        {
            clogr::ScopeTimer<std::chrono::milliseconds> timer("Script runtime load");

            const auto coralDir = R"(C:\Users\alikg\CLionProjects\IonEditor\external\ionEngine\external\coral\Coral.Managed\bin\Debug\net9.0)";
            Coral::HostSettings settings;
            settings.CoralDirectory = coralDir;
            settings.ExceptionCallback = cSharpExceptionCallback;
            Coral::CoralInitStatus status = m_hostInstance.Initialize(settings);

            clogr::ensure(status == Coral::CoralInitStatus::Success, "Coral Initialization Failed with Error: {}", static_cast<int>(status));

            m_loadContext = m_hostInstance.CreateAssemblyLoadContext("LoadContext");

            const auto managedDllPath = R"(C:\Users\alikg\CLionProjects\IonEditor\external\ionEngine\sdk\IonEngine\bin\Debug\net9.0\IonEngine.dll)";
            loadAssembly(managedDllPath, "IonEngine");

            registerModule<EcsScriptModule>("IonEngine");
            registerModule<LoggingScriptModule>("IonEngine");
            registerModule<InputScriptModule>("IonEngine");
        }
    }

    void ScriptingSystem::update()
    {

    }

    void ScriptingSystem::shutdown()
    {

    }

    ScriptAssembly ScriptingSystem::loadAssembly(std::filesystem::path path, const std::string& name)
    {
        const auto& rfs = Engine::resourceFS();
        path = rfs.resolve(path.string()).string();

        auto& assembly = m_loadContext.LoadAssembly(path.string());
        auto scriptAssembly = ScriptAssembly(assembly, path, name);

        m_assemblies.emplace(name, scriptAssembly);

        for(const auto& module : m_assemblyModules[name])
        {
            module->registerInternalCalls(scriptAssembly);
        }

        scriptAssembly.uploadInternalCalls();

        for(const auto& module : m_assemblyModules[name])
        {
            module->onAssemblyLoaded(scriptAssembly);
        }

        return scriptAssembly;
    }


    std::optional<ScriptAssembly> ScriptingSystem::getAssembly(const std::string &name)
    {
        const auto it = m_assemblies.find(name);
        if(it != m_assemblies.end())
            return it->second;

        return std::nullopt;
    }

    bool ScriptingSystem::removeAssembly(const std::string &name)
    {
        const auto it = m_assemblies.find(name);
        if(it == m_assemblies.end())
            return false;

        m_assemblies.erase(it);

        return true;
    }

    void ScriptingSystem::reload()
    {
        m_hostInstance.UnloadAssemblyLoadContext(m_loadContext);
        m_loadContext = m_hostInstance.CreateAssemblyLoadContext("LoadContext");

        for(auto& [name, assembly] : m_assemblies)
        {
            assembly.set(m_loadContext.LoadAssembly(assembly.dllPath().string()));

            for(const auto& module : m_assemblyModules[name])
                module->registerInternalCalls(assembly);

            assembly.uploadInternalCalls();
        }

        for(auto& [name, assembly] : m_assemblies)
        {
            for(const auto& module : m_assemblyModules[name])
                module->onAssemblyLoaded(assembly);
        }
    }
}
