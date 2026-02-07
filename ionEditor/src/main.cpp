
#include "editorSystem.h"
#include "audio/audioSystem.h"
#include "core/coreSystem.h"
#include "core/engine.h"
#include "graphics/imGuiSystem.h"
#include "input/inputSystem.h"
#include "scripting/scriptingSystem.h"
#include "windows/assetsWindow.h"


int main()
{
    ion::EngineConfig config;
    config.windowOptions.title = "Sandbox App";
    config.windowOptions.vsync = false;
    config.windowOptions.fullscreen = false;

    config.resourcePaths.emplace_back(R"(C:\Users\alikg\CLionProjects\ionEngine\ionEngine\resources)");
    config.resourcePaths.emplace_back(R"(C:\Users\alikg\CLionProjects\ionEngine\ionEditor\resources)");
    config.resourcePaths.emplace_back(R"(C:\Users\alikg\CLionProjects\ionEngine\sandboxApp\resources)"); // didnt want to copy the assets over

    ion::Engine::initialize(config);

    ion::Engine::registerSystem<ion::CoreSystem>();
    ion::Engine::registerSystem<ion::AudioSystem>();
    ion::Engine::registerSystem<ion::GraphicsSystem>(config.windowOptions);
    ion::Engine::registerSystem<ion::ImGuiSystem>();
    ion::Engine::registerSystem<ion::InputSystem>();
    ion::Engine::registerSystem<ion::Editor::EditorSystem>();
    ion::Engine::registerSystem<ion::ScriptingSystem>();

    ion::Engine::run();
}
