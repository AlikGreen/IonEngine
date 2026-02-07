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
#include "scriptBridge.h"
#include "Coral/HostInstance.hpp"

namespace ion
{
    void cSharpExceptionCallback(std::string_view errorMessage)
    {
        clogr::error("C# Exception: {}", errorMessage);
    }

    void ScriptingSystem::startup()
    {
        auto& registry = Engine::getSceneManager().getCurrentScene().getRegistry().asTypeErased();
        registry.registerType<Transform>();
        registry.registerType<Camera>();
        registry.registerType<Tag>();
        registry.registerType<MeshRenderer>();

        clogr::info("Tag size {}", sizeof(Tag));

        {
            clogr::ScopeTimer<std::chrono::milliseconds> timer("Script runtime load");

            const auto coralDir = R"(C:\Users\alikg\CLionProjects\IonEngine\ionEngine\dependencies\coral\Coral.Managed\bin\Debug\net9.0)";
            Coral::HostSettings settings;
            settings.CoralDirectory = coralDir;
            settings.ExceptionCallback = cSharpExceptionCallback;
            Coral::HostInstance hostInstance;
            Coral::CoralInitStatus status = hostInstance.Initialize(settings);

            clogr::ensure(status == Coral::CoralInitStatus::Success, "Coral Initialization Failed with Error: {}", (int)status);

            const auto managedDllPath = R"(C:\Users\alikg\CLionProjects\IonEngine\ionEngine\sdk\IonEngine\bin\Debug\net9.0)";
            // std::string testDllPath = exeDir.parent_path().string() + ":" + exeDir.parent_path().parent_path().string();
            auto loadContext = hostInstance.CreateAssemblyLoadContext("TestContext", managedDllPath);

            auto assemblyPath = grl::Path::join(managedDllPath, "IonEngine.dll");
            auto& assembly = loadContext.LoadAssembly(assemblyPath);

            assembly.AddInternalCall("IonEngine.Input", "isKeyHeldCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Input::isKeyHeld));
            assembly.AddInternalCall("IonEngine.Input", "isKeyPressedCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Input::isKeyPressed));
            assembly.AddInternalCall("IonEngine.Input", "isKeyReleasedCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Input::isKeyReleased));

            assembly.AddInternalCall("IonEngine.Scene", "createViewCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::createView));
            assembly.AddInternalCall("IonEngine.Scene", "registerTypeCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::registerComponentType));
            assembly.AddInternalCall("IonEngine.Scene", "getTypeHash", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::getTypeHash));

            assembly.AddInternalCall("IonEngine.SceneManager", "getCurrentRegistryCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::SceneAccess::getSceneRegistry));

            assembly.AddInternalCall("IonEngine.Tag", "getNameCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::ComponentAccessors::Tag_getName));
            assembly.AddInternalCall("IonEngine.Tag", "setNameCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::ComponentAccessors::Tag_setName));

            assembly.AddInternalCall("IonEngine.Log", "logInfoCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::Log::logInfo));

            assembly.AddInternalCall("IonEngine.ViewInterface", "getSizeCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::getViewSize));
            assembly.AddInternalCall("IonEngine.ViewInterface", "getAtIndexCall", reinterpret_cast<void*>(&Scripting::ExportedAPI::EntitySystem::getViewEntry));

            // RegisterTestInternalCalls(assembly);
            assembly.UploadInternalCalls();

            auto& inputClass = assembly.GetLocalType("IonEngine.Input");
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
