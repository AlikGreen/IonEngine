#pragma once
#include <chrono>
#include <memory>
#include <vector>

#include <neonRHI/neonRHI.h>
#include "core/system.h"

namespace ion
{
    class EventManager;
    class AssetManager;
    class AudioManager;
    class SceneManager;

    struct EngineConfig
    {
        std::vector<std::string> resourcePaths{};
        urhi::WindowCreationOptions windowOptions;
    };

    class Engine
    {
    public:
        static void initialize(const EngineConfig &config);
        static void run();

        static void quit();

        template <typename T, typename... Args>
        static T& registerSystem(Args&&... args)
        {
            T* system = new T(std::forward<Args>(args)...);
            registeredSystems.push_back(grl::Box<T>(system));
            return *system;
        }

        template <typename T>
        static T* getSystem()
        {
            for (const auto& system : registeredSystems)
            {
                if (system != nullptr && typeid(*system) == typeid(T))
                {
                    return static_cast<T*>(system.get());
                }
            }
            return nullptr;
        }

        static EngineConfig getConfig();
        static const std::vector<grl::Box<System>>& getSystems();

        static EventManager& getEventManager();
        static AssetManager& getAssetManager();
        static AudioManager& getAudioManager();
        static SceneManager& getSceneManager();

        static float getDeltaTime();
    private:
        static grl::Box<EventManager> eventManager;
        static grl::Box<AssetManager> assetManager;
        static grl::Box<AudioManager> audioManager;
        static grl::Box<SceneManager> sceneManager;

        static std::vector<grl::Box<System>> registeredSystems;
        static EngineConfig config;

        static void shutdown();
        static void startup();

        static bool running;
        static float deltaTime;
    };
}
