#include "gameSystem.h"
#include "core/coreSystem.h"
#include "core/engine.h"
#include "graphics/imGuiSystem.h"
#include "graphics/renderSystem.h"
#include "input/inputSystem.h"


int main()
{
    Neon::EngineConfig config;
    config.windowOptions.title = "Sandbox App";
    config.windowOptions.vsync = false;

    config.engineResourcesPath = R"(C:\Users\alikg\CLionProjects\ionEngine\ionEngine\resources)";
    config.gameResourcesPath = R"(C:\Users\alikg\CLionProjects\ionEngine\sandboxApp\resources)";

    Neon::Engine::initialize(config);

    Neon::Engine::registerSystem<Neon::CoreSystem>();
    Neon::Engine::registerSystem<Neon::GraphicsSystem>(config.windowOptions);
    Neon::Engine::registerSystem<Neon::RenderSystem>();
    Neon::Engine::registerSystem<Neon::ImGuiSystem>();

    Neon::Engine::registerSystem<GameSystem>();

    Neon::Engine::registerSystem<Neon::InputSystem>();

    Neon::Engine::run();
}
