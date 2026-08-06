#include "engine.h"

#include <audio/audioClipImporter.h>


#include "coreSystem.h"

#include "eventManager.h"
#include "sceneManager.h"
#include "resourceFS.h"
#include "asset/assetImportPipeline.h"
#include "asset/assetRegistry.h"
#include "audio/audioManager.h"
#include "graphics/importers/glbSceneImporter.h"
#include "graphics/importers/shaderImporter.h"
#include "serializers/sceneSerializer.h"
#include "graphics/importers/imageImporter.h"
#include "scripting/scriptDllImporter.h"
#include "scripting/scriptManager.h"
#include "serializers/imageSerializer.h"
#include "serializers/materialInstanceSerializer.h"
#include "serializers/materialTemplateSerializer.h"
#include "serializers/meshSerializer.h"
#include "serializers/shaderModuleSerializer.h"
#include "serializers/componentSerializers/meshRendererSerializer.h"
#include "serializers/componentSerializers/parentSerializer.h"
#include "serializers/componentSerializers/pointLightSerializer.h"
#include "serializers/componentSerializers/scriptComponentSerializer.h"
#include "serializers/componentSerializers/tagSerializer.h"
#include "serializers/componentSerializers/transformSerializer.h"

namespace ion
{
    grl::Box<EventManager> Engine::m_eventManager;
    grl::Box<AudioManager> Engine::m_audioManager;
    grl::Box<SceneManager> Engine::m_sceneManager;
    grl::Box<ResourceFS>   Engine::m_resourceFS;
    grl::Box<AssetRegistry> Engine::m_assetRegistry;
    grl::Box<AssetImportPipeline> Engine::m_assetImportPipeline;
    grl::Box<ScriptManager> Engine::m_scriptManager;

    bool Engine::m_running = false;
    float Engine::m_deltaTime = 0.0f;
    double Engine::m_time = 0.0f;
    uint64_t Engine::m_frames = 0;

    std::vector<grl::Box<System>> Engine::m_systems{};

    std::vector<System*> Engine::m_preStartupSystems{}, Engine::m_startupSystems{}, Engine::m_postStartupSystems{};
    std::vector<System*> Engine::m_preShutdownSystems{}, Engine::m_shutdownSystems{}, Engine::m_postShutdownSystems{};

    std::vector<System*> Engine::m_preUpdateSystems{}, Engine::m_updateSystems{}, Engine::m_postUpdateSystems{};
    std::vector<System*> Engine::m_preUpdateSceneSystems{}, Engine::m_updateSceneSystems{}, Engine::m_postUpdateSceneSystems{};

    std::vector<System*> Engine::m_preRenderSystems{}, Engine::m_renderSystems{}, Engine::m_postRenderSystems{};
    std::vector<System*> Engine::m_preRenderSceneSystems{}, Engine::m_renderSceneSystems{}, Engine::m_postRenderSceneSystems{};

    std::vector<System*> Engine::m_sceneLoadedSystems{}, Engine::m_sceneUnloadedSystems{};
    std::vector<System*> Engine::m_eventSystems{};

    std::unordered_map<System*, uint64_t> Engine::m_systemTags;

    void Engine::initialize()
    {
        m_resourceFS = grl::makeBox<ResourceFS>();
        m_assetRegistry = grl::makeBox<AssetRegistry>();
        m_assetImportPipeline = grl::makeBox<AssetImportPipeline>(*m_assetRegistry, *m_resourceFS);

        m_eventManager = grl::makeBox<EventManager>();
        m_audioManager = grl::makeBox<AudioManager>();
        m_sceneManager = grl::makeBox<SceneManager>(m_sceneLoadedSystems, m_sceneUnloadedSystems);

        m_scriptManager = grl::makeBox<ScriptManager>();

        m_assetImportPipeline->registerImporter<GLBSceneImporter>();
        m_assetImportPipeline->registerImporter<ImageImporter>();
        m_assetImportPipeline->registerImporter<ShaderImporter>();
        m_assetImportPipeline->registerImporter<AudioClipImporter>();
        m_assetImportPipeline->registerImporter<ScriptDllImporter>();

        auto& sceneSerializer = m_assetRegistry->registerSerializer<SceneSerializer>();
        sceneSerializer.registerComponentSerializer<TagSerializer>();
        sceneSerializer.registerComponentSerializer<TransformSerializer>();
        sceneSerializer.registerComponentSerializer<ParentSerializer>();
        sceneSerializer.registerComponentSerializer<MeshRendererSerializer>();
        sceneSerializer.registerComponentSerializer<PointLightSerializer>();
        sceneSerializer.registerComponentSerializer<ScriptComponentSerializer>();

        m_assetRegistry->registerSerializer<MeshSerializer>();
        m_assetRegistry->registerSerializer<ImageSerializer>();
        m_assetRegistry->registerSerializer<MaterialInstanceSerializer>();
        m_assetRegistry->registerSerializer<MaterialTemplateSerializer>();
        m_assetRegistry->registerSerializer<ShaderModuleSerializer>();
    }


    void Engine::run()
    {
        m_scriptManager->init();

        startup();

        m_running = true;
        while (m_running)
        {
            auto start = std::chrono::high_resolution_clock::now();

            m_eventManager->handleEvents();

            auto scene = sceneManager().activeScene();

            update(scene.get());
            render(scene.get());

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> diff = end - start;
            m_deltaTime = diff.count();
            m_time += m_deltaTime;
            m_frames++;
        }

        shutdown();
    }

    void Engine::startup()
    {
        for (auto* s: m_preStartupSystems) s->preStartup();
        for (auto* s: m_startupSystems) s->startup();
        for (auto* s: m_postStartupSystems) s->postStartup();
    }

    void Engine::update(Scene* scene)
    {
        m_sceneManager->loadScene();

        tick(m_preUpdateSystems,  [](auto& s) { s.preUpdate(); });
        if (scene) tick(m_preUpdateSceneSystems,  [scene](auto& s) { s.preUpdate(*scene); });

        tick(m_updateSystems,     [](auto& s) { s.update(); });
        if (scene) tick(m_updateSceneSystems,  [scene](auto& s) { s.update(*scene); });

        tick(m_postUpdateSystems, [](auto& s) { s.postUpdate(); });
        if (scene) tick(m_postUpdateSceneSystems,  [scene](auto& s) { s.postUpdate(*scene); });
    }

    void Engine::render(Scene *scene)
    {
        tick(m_preRenderSystems,  [](auto& s) { s.preRender(); });
        if (scene) tick(m_preRenderSceneSystems,  [scene](auto& s) { s.preRender(*scene); });

        tick(m_renderSystems,     [](auto& s) { s.render(); });
        if (scene) tick(m_renderSceneSystems,  [scene](auto& s) { s.render(*scene); });

        tick(m_postRenderSystems, [](auto& s) { s.postRender(); });
        if (scene) tick(m_postRenderSceneSystems,  [scene](auto& s) { s.postRender(*scene); });
    }

    void Engine::shutdown()
    {
        for (auto* s : m_preShutdownSystems)  s->preShutdown();
        for (auto* s : m_shutdownSystems)     s->shutdown();
        for (auto* s : m_postShutdownSystems) s->postShutdown();
    }
}
