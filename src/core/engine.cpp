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
#include "serializers/meshSerializer.h"
#include "serializers/componentSerializers/meshRendererSerializer.h"
#include "serializers/componentSerializers/parentSerializer.h"
#include "serializers/componentSerializers/pointLightSerializer.h"
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

    std::vector<grl::Box<System>> Engine::m_registeredSystems{};

    void Engine::initialize()
    {
        m_resourceFS = grl::makeBox<ResourceFS>();
        m_assetRegistry = grl::makeBox<AssetRegistry>();
        m_assetImportPipeline = grl::makeBox<AssetImportPipeline>(*m_assetRegistry, *m_resourceFS);

        m_eventManager = grl::makeBox<EventManager>();
        m_audioManager = grl::makeBox<AudioManager>();
        m_sceneManager = grl::makeBox<SceneManager>(m_registeredSystems);

        m_scriptManager = grl::makeBox<ScriptManager>();

        m_assetImportPipeline->registerImporter<GLBSceneImporter>();
        m_assetImportPipeline->registerImporter<ImageImporter>();
        m_assetImportPipeline->registerImporter<ShaderImporter>();
        m_assetImportPipeline->registerImporter<AudioClipImporter>();
        m_assetImportPipeline->registerImporter<ScriptDllImporter>();

        auto& sceneSerializer = m_assetRegistry->registerSerializer<SceneSerializer>();
        sceneSerializer.registerComponentSerializer<grl::hash32("ion::Tag"), TagSerializer>();
        sceneSerializer.registerComponentSerializer<grl::hash32("ion::Transform"), TransformSerializer>();
        sceneSerializer.registerComponentSerializer<grl::hash32("ion::Parent"), ParentSerializer>();
        sceneSerializer.registerComponentSerializer<grl::hash32("ion::MeshRenderer"), MeshRendererSerializer>();
        sceneSerializer.registerComponentSerializer<grl::hash32("ion::PointLight"), PointLightSerializer>();

        m_assetRegistry->registerSerializer<MeshSerializer>();
    }


    void Engine::run()
    {
        m_sceneManager->addScene("DefaultScene", assetRegistry().create<Scene>());
        m_sceneManager->setScene("DefaultScene");

        m_scriptManager->init();
        startup();

        m_running = true;
        while (m_running)
        {
            auto start = std::chrono::high_resolution_clock::now();

            m_eventManager->handleEvents();

            // Update
            for (const auto& system: m_registeredSystems)
            {
                system->preUpdate();
            }

            for (const auto& system: m_registeredSystems)
            {
                system->update();
            }

            for (const auto& system: m_registeredSystems)
            {
                system->postUpdate();
            }


            // Render
            for (const auto& system: m_registeredSystems)
            {
                system->preRender();
            }

            for (const auto& system: m_registeredSystems)
            {
                system->render();
            }

            for (const auto& system: m_registeredSystems)
            {
                system->postRender();
            }

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

        for (const auto& system: m_registeredSystems)
        {
            system->preStartup();
        }

        for (const auto& system: m_registeredSystems)
        {
            system->startup();
        }

        for (const auto& system: m_registeredSystems)
        {
            system->postStartup();
        }
    }

    void Engine::shutdown()
    {
        for (const auto& system: m_registeredSystems)
        {
            system->preShutdown();
        }

        for (const auto& system: m_registeredSystems)
        {
            system->shutdown();
        }

        for (const auto& system: m_registeredSystems)
        {
            system->postShutdown();
        }
    }
}
