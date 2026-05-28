#include "scriptSystem.h"

#include "scriptComponent.h"
#include "core/engine.h"
#include "core/sceneManager.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/components/meshRenderer.h"


namespace ion
{
    void ScriptSystem::sceneLoaded(Scene &scene)
    {
        auto* ctx = Engine::scriptManager().getContext("UserScripts");
        if(!ctx) return;

        auto& registry = scene.registry();
        registry.asTypeErased().registerType<Transform>();
        registry.asTypeErased().registerType<Camera>();
        registry.asTypeErased().registerType<Tag>();
        registry.asTypeErased().registerType<MeshRenderer>();

        for(const auto& [entity, comp] : registry.view<ScriptComponent>())
        {
            comp.reload(*ctx, entity);
        }

        for(auto [entity, scriptComponent] : registry.view<ScriptComponent>())
        {
            for(auto& script : scriptComponent.scripts)
            {
                if(script.object().IsValid())
                    script.object().InvokeMethod("Start");
            }
        }
    }

    void ScriptSystem::sceneUnloaded(Scene &scene)
    {
        for(const auto& [entity, comp] : scene.registry().view<ScriptComponent>())
        {
            comp.unload();
        }
    }

    void ScriptSystem::update(Scene& scene)
    {
        auto& registry = scene.registry();
        for(auto [entity, scriptComponent] : registry.view<ScriptComponent>())
        {
            for(auto& script : scriptComponent.scripts)
            {
                if(script.object().IsValid())
                    script.object().InvokeMethod("Update");
            }
        }
    }
}
