#include "scriptingSystem.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <neonECS/neonECS.h>

#include "core/engine.h"
#include "core/sceneManager.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/components/meshRenderer.h"
#include "timers/scopeTimer.h"
#include "log.h"
#include "scriptBridge.h"
#include "Coral/HostInstance.hpp"

namespace Neon
{
    void cSharpExceptionCallback(std::string_view errorMessage)
    {
        Log::error("C# Exception: {}", errorMessage);
    }

    void ScriptingSystem::startup()
    {
        auto& registry = Engine::getSceneManager().getCurrentScene().getRegistry().asTypeErased();
        registry.registerType<Transform>();
        registry.registerType<Camera>();
        registry.registerType<Tag>();
        registry.registerType<MeshRenderer>();

        Log::info("Tag size {}", sizeof(Tag));

        {
            Debug::ScopeTimer<std::chrono::milliseconds> timer("Script runtime load");

            const auto coralDir = R"(C:\Users\alikg\CLionProjects\NeonEngine\neonEngine\dependencies\coral\Coral.Managed\bin\Debug\net9.0)";
            Coral::HostSettings settings;
            settings.CoralDirectory = coralDir;
            settings.ExceptionCallback = cSharpExceptionCallback;
            Coral::HostInstance hostInstance;
            Coral::CoralInitStatus status = hostInstance.Initialize(settings);

            Debug::ensure(status == Coral::CoralInitStatus::Success, "Coral Initialization Failed with Error: {}", (int)status);

            const auto managedDllPath = R"(C:\Users\alikg\CLionProjects\NeonEngine\neonEngine\sdk\NeonEngine\bin\Debug\net9.0)";
            // std::string testDllPath = exeDir.parent_path().string() + ":" + exeDir.parent_path().parent_path().string();
            auto loadContext = hostInstance.CreateAssemblyLoadContext("TestContext", managedDllPath);

            auto assemblyPath = Path::join(managedDllPath, "NeonEngine.dll");
            auto& assembly = loadContext.LoadAssembly(assemblyPath);

            assembly.AddInternalCall("NeonEngine.Input", "isKeyHeldCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Input::isKeyHeld));
            assembly.AddInternalCall("NeonEngine.Input", "isKeyPressedCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Input::isKeyPressed));
            assembly.AddInternalCall("NeonEngine.Input", "isKeyReleasedCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Input::isKeyReleased));

            assembly.AddInternalCall("NeonEngine.Scene", "createViewCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::createView));
            assembly.AddInternalCall("NeonEngine.Scene", "registerTypeCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::registerComponentType));
            assembly.AddInternalCall("NeonEngine.Scene", "getTypeHash", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::getTypeHash));

            assembly.AddInternalCall("NeonEngine.SceneManager", "getCurrentRegistryCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::SceneAccess::getSceneRegistry));

            assembly.AddInternalCall("NeonEngine.Tag", "getNameCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::ComponentAccessors::Tag_getName));
            assembly.AddInternalCall("NeonEngine.Tag", "setNameCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::ComponentAccessors::Tag_setName));

            assembly.AddInternalCall("NeonEngine.Log", "logInfoCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Log::logInfo));

            assembly.AddInternalCall("NeonEngine.ViewInterface", "getSizeCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::getViewSize));
            assembly.AddInternalCall("NeonEngine.ViewInterface", "getAtIndexCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::getViewEntry));

            // RegisterTestInternalCalls(assembly);
            assembly.UploadInternalCalls();

            auto& inputClass = assembly.GetLocalType("NeonEngine.Input");
            inputClass.InvokeStaticMethod("Run");
        }
    }

    void ScriptingSystem::update()
    {

    }

    void ScriptingSystem::shutdown()
    {

    }
}
