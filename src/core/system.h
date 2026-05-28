#pragma once
#include "event.h"
#include "scene.h"

namespace ion
{
    class System
    {
    public:
        virtual ~System() = default;

        virtual void preStartup() { }
        virtual void startup() { }
        virtual void postStartup() { }

        virtual void preShutdown() {}
        virtual void shutdown()    {}
        virtual void postShutdown(){}

        virtual void preUpdate(Scene& scene) { }
        virtual void update(Scene& scene) { }
        virtual void postUpdate(Scene& scene) { }

        virtual void preUpdate() { }
        virtual void update() { }
        virtual void postUpdate() { }

        virtual void preRender(Scene& scene) { }
        virtual void render(Scene& scene) { }
        virtual void postRender(Scene& scene) { }

        virtual void preRender() { }
        virtual void render() { }
        virtual void postRender() { }

        virtual void sceneLoaded(Scene& scene)   {}
        virtual void sceneUnloaded(Scene& scene) {}

        virtual void enabled()  {}
        virtual void disabled() {}

        virtual void event(Event* event) { }

        bool isEnabled = true;
    };
}
