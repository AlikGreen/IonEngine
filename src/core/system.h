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

        virtual void preUpdate() { }
        virtual void update() { }
        virtual void postUpdate() { }

        virtual void preRender() { }
        virtual void render() { }
        virtual void postRender() { }

        virtual void sceneLoaded(Scene& scene)   {}
        virtual void sceneUnloaded(Scene& scene) {}

        virtual void event(Event* event) { }
    };
}
