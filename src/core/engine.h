#pragma once
#include <memory>
#include <vector>

#include <urhi/urhi.h>

#include "core/system.h"
#include "descriptions/windowDesc.h"

namespace ion
{
    class EventManager;
    class AudioManager;
    class SceneManager;
    class ResourceFS;
    class AssetImportPipeline;
    class AssetRegistry;


    class Engine
    {
    public:
        static void initialize();
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

        static const std::vector<grl::Box<System>>& getSystems();

        static EventManager&  eventManager();
        static AudioManager&  audioManager();
        static SceneManager&  sceneManager();
        static ResourceFS&    resourceFS();
        static AssetRegistry& assetRegistry();
        static AssetImportPipeline& assetImportPipeline();

        static float getDeltaTime();
        static double getTime();
        static uint64_t getFrames();

        static constexpr std::string version() { return "0.0.1"; }
    private:
        static grl::Box<EventManager> m_eventManager;
        static grl::Box<AssetRegistry> m_assetRegistry;
        static grl::Box<AudioManager> m_audioManager;
        static grl::Box<SceneManager> m_sceneManager;
        static grl::Box<ResourceFS> m_resourceFS;
        static grl::Box<AssetImportPipeline> m_assetImportPipeline;

        static std::vector<grl::Box<System>> registeredSystems;

        static void shutdown();
        static void startup();

        static bool running;
        static float deltaTime;
        static double time;
        static uint64_t frames;
    };
}
