#pragma once
#include <memory>
#include <vector>

#include "core/system.h"
#include "scripting/scriptManager.h"

template<typename T>
concept TagEnum = std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, uint64_t>;


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

        template<typename T, TagEnum TTag, typename... Args>
        static T& registerSystem(TTag tags, Args&&... args)
        {
            static_assert(std::is_base_of_v<System, T>, "T must derive from System");

            auto owned = std::make_unique<T>(std::forward<Args>(args)...);
            T* s = owned.get();

            m_systemTags[s] = static_cast<uint64_t>(tags);

            auto add = [&]<typename Sig>(std::vector<System*>& list, Sig T::* derived, Sig System::* base)
            {
                if (derived != base)
                    list.push_back(s);
            };

            if constexpr (requires { static_cast<void(T::*)()>(&T::preStartup); })
                add(m_preStartupSystems,  static_cast<void(T::*)()>(&T::preStartup),  &System::preStartup);
            if constexpr (requires { static_cast<void(T::*)()>(&T::startup); })
                add(m_startupSystems,     static_cast<void(T::*)()>(&T::startup),     &System::startup);
            if constexpr (requires { static_cast<void(T::*)()>(&T::postStartup); })
                add(m_postStartupSystems, static_cast<void(T::*)()>(&T::postStartup), &System::postStartup);

            if constexpr (requires { static_cast<void(T::*)()>(&T::preShutdown); })
                add(m_preShutdownSystems,  static_cast<void(T::*)()>(&T::preShutdown),  &System::preShutdown);
            if constexpr (requires { static_cast<void(T::*)()>(&T::shutdown); })
                add(m_shutdownSystems,     static_cast<void(T::*)()>(&T::shutdown),     &System::shutdown);
            if constexpr (requires { static_cast<void(T::*)()>(&T::postShutdown); })
                add(m_postShutdownSystems, static_cast<void(T::*)()>(&T::postShutdown), &System::postShutdown);

            if constexpr (requires { static_cast<void(T::*)()>(&T::preUpdate); })
                add(m_preUpdateSystems, static_cast<void(T::*)()>(&T::preUpdate), &System::preUpdate);
            if constexpr (requires { static_cast<void(T::*)()>(&T::update); })
                add(m_updateSystems,    static_cast<void(T::*)()>(&T::update),    &System::update);
            if constexpr (requires { static_cast<void(T::*)()>(&T::postUpdate); })
                add(m_postUpdateSystems,static_cast<void(T::*)()>(&T::postUpdate),&System::postUpdate);

            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::preUpdate); })
                add(m_preUpdateSceneSystems, static_cast<void(T::*)(Scene&)>(&T::preUpdate), &System::preUpdate);
            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::update); })
                add(m_updateSceneSystems,    static_cast<void(T::*)(Scene&)>(&T::update),    &System::update);
            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::postUpdate); })
                add(m_postUpdateSceneSystems,static_cast<void(T::*)(Scene&)>(&T::postUpdate),&System::postUpdate);

            if constexpr (requires { static_cast<void(T::*)()>(&T::preRender); })
                add(m_preRenderSystems, static_cast<void(T::*)()>(&T::preRender), &System::preRender);
            if constexpr (requires { static_cast<void(T::*)()>(&T::render); })
                add(m_renderSystems,    static_cast<void(T::*)()>(&T::render),    &System::render);
            if constexpr (requires { static_cast<void(T::*)()>(&T::postRender); })
                add(m_postRenderSystems,static_cast<void(T::*)()>(&T::postRender),&System::postRender);

            // Render — void(Scene&)
            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::preRender); })
                add(m_preRenderSceneSystems, static_cast<void(T::*)(Scene&)>(&T::preRender), &System::preRender);
            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::render); })
                add(m_renderSceneSystems,    static_cast<void(T::*)(Scene&)>(&T::render),    &System::render);
            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::postRender); })
                add(m_postRenderSceneSystems,static_cast<void(T::*)(Scene&)>(&T::postRender),&System::postRender);

            // Scene events
            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::sceneLoaded); })
                add(m_sceneLoadedSystems,   static_cast<void(T::*)(Scene&)>(&T::sceneLoaded),   &System::sceneLoaded);
            if constexpr (requires { static_cast<void(T::*)(Scene&)>(&T::sceneUnloaded); })
                add(m_sceneUnloadedSystems, static_cast<void(T::*)(Scene&)>(&T::sceneUnloaded), &System::sceneUnloaded);

            // Events
            if constexpr (requires { static_cast<void(T::*)(Event*)>(&T::event); })
                add(m_eventSystems, static_cast<void(T::*)(Event*)>(&T::event), &System::event);

            m_systems.push_back(std::move(owned));
            return *s;
        }

        template <typename T>
        static T* getSystem()
        {
            for (const auto& system : m_systems)
            {
                if (system != nullptr && typeid(*system) == typeid(T))
                {
                    return static_cast<T*>(system.get());
                }
            }
            return nullptr;
        }

        static const std::vector<grl::Box<System>>& getSystems() { return m_systems; }

        template<TagEnum TTag>
        static void disableTag(TTag tag)
        {
           for(const auto& system : m_systems)
           {
               const auto systemTag = m_systemTags.at(system.get());
               if(systemTag & static_cast<uint64_t>(tag) != 0 && system->isEnabled)
               {
                   system->isEnabled = false;
                   system->disabled();
               }
           }
        }

        template<TagEnum TTag>
        static void enableTag(TTag tag)
        {
            for(const auto& system : m_systems)
            {
                const auto systemTag = m_systemTags.at(system.get());
                if(systemTag & static_cast<uint64_t>(tag) != 0 && !system->isEnabled)
                {
                    system->isEnabled = true;
                    system->enabled();
                }
            }
        }


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
        template<typename Fn>
        static void tick(const std::vector<System*>& systems, Fn&& fn)
        {
            for (auto* s : systems)
                if (s->isEnabled) fn(*s);
        }

        static void startup();
        static void update(Scene* scene);
        static void render(Scene* scene);
        static void shutdown();

        static grl::Box<EventManager> m_eventManager;
        static grl::Box<AssetRegistry> m_assetRegistry;
        static grl::Box<AudioManager> m_audioManager;
        static grl::Box<SceneManager> m_sceneManager;
        static grl::Box<ResourceFS> m_resourceFS;
        static grl::Box<AssetImportPipeline> m_assetImportPipeline;
        static grl::Box<ScriptManager> m_scriptManager;

        // All systems
        static std::vector<grl::Box<System>> m_systems;

        static std::vector<System*> m_preStartupSystems,  m_startupSystems,  m_postStartupSystems;
        static std::vector<System*> m_preShutdownSystems, m_shutdownSystems, m_postShutdownSystems;

        static std::vector<System*> m_preUpdateSystems,  m_preUpdateSceneSystems;
        static std::vector<System*> m_updateSystems,     m_updateSceneSystems;
        static std::vector<System*> m_postUpdateSystems, m_postUpdateSceneSystems;

        static std::vector<System*> m_preRenderSystems,  m_preRenderSceneSystems;
        static std::vector<System*> m_renderSystems,     m_renderSceneSystems;
        static std::vector<System*> m_postRenderSystems, m_postRenderSceneSystems;

        static std::vector<System*> m_sceneLoadedSystems, m_sceneUnloadedSystems;
        static std::vector<System*> m_eventSystems;

        static std::unordered_map<System*, uint64_t> m_systemTags;

        static bool m_running;
        static float m_deltaTime;
        static double m_time;
        static uint64_t m_frames;
    };
}
