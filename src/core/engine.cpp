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

    bool Engine::running = false;
    float Engine::deltaTime = 0.0f;
    std::vector<grl::Box<System>> Engine::registeredSystems{};
    EngineConfig Engine::config;

    void Engine::initialize(const EngineConfig &config)
    {
        Engine::config = config;

        m_resourceFS = grl::makeBox<ResourceFS>();
        m_assetRegistry = grl::makeBox<AssetRegistry>();
        m_assetImportPipeline = grl::makeBox<AssetImportPipeline>(*m_assetRegistry, *m_resourceFS);

        m_eventManager = grl::makeBox<EventManager>();
        m_audioManager = grl::makeBox<AudioManager>();
        m_sceneManager = grl::makeBox<SceneManager>();

        m_assetImportPipeline->registerImporter<GLBSceneImporter>();
        m_assetImportPipeline->registerImporter<ImageImporter>();
        m_assetImportPipeline->registerImporter<ShaderImporter>();
        m_assetImportPipeline->registerImporter<AudioClipImporter>();

        auto& sceneSerializer = m_assetRegistry->registerSerializer<Scene, SceneSerializer>();
        sceneSerializer.registerComponentSerializer<Tag, TagSerializer>(grl::hash32("ion::Tag"));
        sceneSerializer.registerComponentSerializer<Transform, TransformSerializer>(grl::hash32("ion::Transform"));
        sceneSerializer.registerComponentSerializer<Parent, ParentSerializer>(grl::hash32("ion::Parent"));
        sceneSerializer.registerComponentSerializer<MeshRenderer, MeshRendererSerializer>(grl::hash32("ion::MeshRenderer"));
        sceneSerializer.registerComponentSerializer<PointLight, PointLightSerializer>(grl::hash32("ion::PointLight"));

        m_assetRegistry->registerSerializer<Mesh, MeshSerializer>();
    }

    void Engine::quit()
    {
        running = false;
    }

    EngineConfig Engine::getConfig()
    {
        return config;
    }

    const std::vector<grl::Box<System>>& Engine::getSystems()
    {
        return registeredSystems;
    }

    EventManager& Engine::eventManager()
    {
        return *m_eventManager;
    }


    AudioManager& Engine::audioManager()
    {
        return *m_audioManager;
    }

    SceneManager& Engine::sceneManager()
    {
        return *m_sceneManager;
    }

    ResourceFS& Engine::resourceFS()
    {
        return *m_resourceFS;
    }

    AssetRegistry& Engine::assetRegistry()
    {
        return *m_assetRegistry;
    }

    AssetImportPipeline& Engine::assetImportPipeline()
    {
        return *m_assetImportPipeline;
    }

    float Engine::getDeltaTime()
    {
        return deltaTime;
    }

    void Engine::run()
    {
        startup();

        running = true;
        while (running)
        {
            auto start = std::chrono::high_resolution_clock::now();

            m_eventManager->handleEvents();

            for (const auto& system: registeredSystems)
            {
                system->preUpdate();
            }

            for (const auto& system: registeredSystems)
            {
                system->update();
            }

            for (const auto& system: registeredSystems)
            {
                system->postUpdate();
            }

            for (const auto& system: registeredSystems)
            {
                system->preRender();
            }

            for (const auto& system: registeredSystems)
            {
                system->render();
            }

            for (const auto& system: registeredSystems)
            {
                system->postRender();
            }

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> diff = end - start;
            deltaTime = diff.count();
        }

        shutdown();
    }

    void Engine::startup()
    {

        for (const auto& system: registeredSystems)
        {
            system->preStartup();
        }

        for (const auto& system: registeredSystems)
        {
            system->startup();
        }

        for (const auto& system: registeredSystems)
        {
            system->postStartup();
        }
    }

    void Engine::shutdown()
    {
        for (const auto& system: registeredSystems)
        {
            system->shutdown();
        }
    }
}
