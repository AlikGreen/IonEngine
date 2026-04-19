#pragma once
#include <memory>
#include <vector>

#include <urhi/urhi.h>

#include "core/system.h"
#include "scripting/scriptManager.h"

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

        static void quit() { m_running = false; }

        template <typename T, typename... Args>
        static T& registerSystem(Args&&... args)
        {
            T* system = new T(std::forward<Args>(args)...);
            m_registeredSystems.push_back(grl::Box<T>(system));
            return *system;
        }

        template <typename T>
        static T* getSystem()
        {
            for (const auto& system : m_registeredSystems)
            {
                if (system != nullptr && typeid(*system) == typeid(T))
                {
                    return static_cast<T*>(system.get());
                }
            }
            return nullptr;
        }

        static const std::vector<grl::Box<System>>& getSystems() { return m_registeredSystems; }

        static EventManager&  eventManager() { return *m_eventManager; }
        static AudioManager&  audioManager() { return *m_audioManager; }
        static SceneManager&  sceneManager() { return *m_sceneManager; }
        static ResourceFS&    resourceFS() { return *m_resourceFS; }
        static AssetRegistry& assetRegistry() { return *m_assetRegistry; }
        static AssetImportPipeline& assetImportPipeline() { return *m_assetImportPipeline; }
        static ScriptManager& scriptManager() { return *m_scriptManager; }

        static float getDeltaTime() { return m_deltaTime; }
        static double getTime() { return m_time; }
        static uint64_t getFrames() { return m_frames; }

        static constexpr std::string version() { return "0.0.1"; }
    private:
        // Probably shouldn't be m_
        static grl::Box<EventManager> m_eventManager;
        static grl::Box<AssetRegistry> m_assetRegistry;
        static grl::Box<AudioManager> m_audioManager;
        static grl::Box<SceneManager> m_sceneManager;
        static grl::Box<ResourceFS> m_resourceFS;
        static grl::Box<AssetImportPipeline> m_assetImportPipeline;
        static grl::Box<ScriptManager> m_scriptManager;

        static std::vector<grl::Box<System>> m_registeredSystems;

        static void shutdown();
        static void startup();

        static bool m_running;
        static float m_deltaTime;
        static double m_time;
        static uint64_t m_frames;
    };
}
