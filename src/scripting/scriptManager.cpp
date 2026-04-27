#include "scriptManager.h"

#include "asset/assetImportPipeline.h"
#include "bindings/ecsScriptBindings.h"
#include "bindings/inputScriptBindings.h"
#include "bindings/loggingScriptBindings.h"
#include "core/engine.h"

namespace ion
{
    void cSharpExceptionCallback(std::string_view errorMessage)
    {
        clogr::error("C# Exception: {}", errorMessage);
    }

    void ScriptManager::init()
    {
        const auto coralDir = R"(C:\Users\alikg\CLionProjects\IonEditor\external\ionEngine\external\coral\Coral.Managed\bin\Debug\net9.0)";
        Coral::HostSettings settings;
        settings.CoralDirectory = coralDir;
        settings.ExceptionCallback = cSharpExceptionCallback;
        Coral::CoralInitStatus status = m_hostInstance.Initialize(settings);

        clogr::ensure(status == Coral::CoralInitStatus::Success, "Coral Initialization Failed with Error: {}", static_cast<int>(status));

        const std::string ionEngineDllPath = R"(C:\Users\alikg\CLionProjects\IonEditor\external\ionEngine\sdk\IonEngine\IonEngine\bin\Debug\net9.0\IonEngine.dll)";
        auto& importPipeline = Engine::assetImportPipeline();
        m_engineAssemblyData = importPipeline.import<ScriptAssemblyData>(ionEngineDllPath);
    }

    ScriptContext& ScriptManager::createContext(const std::string &name)
    {
        m_scriptContexts[name] = ScriptContext{m_hostInstance, m_engineAssemblyData, name};
        return m_scriptContexts[name];
    }

    void ScriptManager::unloadContext(const std::string &name)
    {
        m_scriptContexts.erase(name);
    }

    ScriptContext* ScriptManager::getContext(const std::string &name)
    {
        const auto it = m_scriptContexts.find(name);
        if(it == m_scriptContexts.end())
            return nullptr;

        return &it->second;
    }
}
