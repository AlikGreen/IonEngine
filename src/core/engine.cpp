#include "engine.h"

#include <audio/audioClipImporter.h>


#include "coreSystem.h"

#include "eventManager.h"
#include "sceneManager.h"
#include "audio/audioManager.h"
#include "asset/assetManager.h"
#include "graphics/importers/glbSceneImporter.h"
#include "graphics/importers/shaderImporter.h"
#include "graphics/importers/textureImporter.h"
#include "core/serializers/sceneSerializer.h"
#include "serializers/componentSerializers/parentSerializer.h"
#include "serializers/componentSerializers/tagSerializer.h"
#include "serializers/componentSerializers/transformSerializer.h"

namespace ion
{
    grl::Box<EventManager> Engine::eventManager;
    grl::Box<AssetManager> Engine::assetManager;
    grl::Box<AudioManager> Engine::audioManager;
    grl::Box<SceneManager> Engine::sceneManager;

    bool Engine::running = false;
    float Engine::deltaTime = 0.0f;
    std::vector<grl::Box<System>> Engine::registeredSystems{};
    EngineConfig Engine::config;

    void Engine::initialize(const EngineConfig &config)
    {
        Engine::config = config;
        eventManager = grl::makeBox<EventManager>();
        assetManager = grl::makeBox<AssetManager>();
        audioManager = grl::makeBox<AudioManager>();
        sceneManager = grl::makeBox<SceneManager>();

        assetManager->registerImporter<GLBSceneImporter, Scene>();
        assetManager->registerImporter<TextureImporter, TextureData>();
        assetManager->registerImporter<ShaderImporter, grl::Rc<urhi::Shader>>();
        assetManager->registerImporter<AudioClipImporter, AudioClip>();

        // component serializers
        assetManager->registerSerializer<TagSerializer, Tag>();
        assetManager->registerSerializer<TransformSerializer, Transform>();
        assetManager->registerSerializer<ParentSerializer, Parent>();

        auto& sceneSerializer = assetManager->registerSerializer<SceneSerializer, Scene>();
        sceneSerializer.registerComponentSerializer<Tag>(1);
        sceneSerializer.registerComponentSerializer<Transform>(2);
        sceneSerializer.registerComponentSerializer<Parent>(3);
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

    EventManager& Engine::getEventManager()
    {
        return *eventManager;
    }

    AssetManager& Engine::getAssetManager()
    {
        return *assetManager;
    }

    AudioManager& Engine::getAudioManager()
    {
        return *audioManager;
    }

    SceneManager& Engine::getSceneManager()
    {
        return *sceneManager;
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

            eventManager->handleEvents();

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
