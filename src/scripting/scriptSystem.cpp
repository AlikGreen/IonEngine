#include "scriptSystem.h"

#include "scriptComponent.h"
#include "core/engine.h"
#include "core/sceneManager.h"


namespace ion
{
    void ScriptSystem::sceneLoaded(Scene &scene)
    {
        auto* ctx = Engine::scriptManager().getContext("UserScripts");
        if(!ctx) return;

        auto& registry = Engine::sceneManager().getCurrentScene().registry();

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

    void ScriptSystem::update()
    {
        auto& registry = Engine::sceneManager().getCurrentScene().registry();
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
