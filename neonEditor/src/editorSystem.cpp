#include "editorSystem.h"


#include "editorCamera.h"
#include "audio/components/audioSource.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/image.h"
#include "graphics/imGuiSystem.h"
#include "graphics/events/dropFileEvent.h"
#include "neonEngine/neonEngine.h"
#include "windows/assetsWindow.h"
#include "windows/propertiesWindow.h"
#include "windows/sceneGraphWindow.h"

#include <neonCore/neonCore.h>

namespace Neon::Editor
{
    void EditorSystem::startup()
    {
        auto& scene = Engine::getSceneManager().getCurrentScene();

        AssetManager& assetManager = Engine::getAssetManager();
        auto model = assetManager.import<Scene>("models/monkey.glb");

        ECS::Entity modelEntity = scene.import(*model);
        modelEntity.get<Tag>().name = "Imported model";
        modelEntity.get<Transform>().setScale(glm::vec3(1.0f));


        auto skyboxData =  assetManager.loadUnmanaged<TextureData>("textures/skybox.hdr");
        const Rc<RHI::Texture> skyboxTexture = assetManager.addAsset(Image(skyboxData.release()), "Skybox Image")->texture;
        const auto skyboxTexViewDesc = RHI::TextureViewDesc(skyboxTexture);
        const Rc<RHI::TextureView> skyboxTextureView = Engine::getSystem<GraphicsSystem>()->getDevice()->createTextureView(skyboxTexViewDesc);

        MaterialShader skyboxMaterial = MaterialShader::createEquirectangularSkybox();
        skyboxMaterial.setTexture("skyboxTexture", skyboxTextureView);
        const AssetRef<MaterialShader> skyboxMaterialAsset = assetManager.addAsset<MaterialShader>(skyboxMaterial);

        // Player/Camera entity
        ECS::Entity cameraEntity = scene.createEntity("Main Camera");
        auto& camera = cameraEntity.emplace<Camera>();
        camera.skyboxMaterial = skyboxMaterialAsset;
        auto& camTransform = cameraEntity.get<Transform>();
        camTransform.setPosition({0, 0, 0});

        ECS::Entity editorCameraEntity = scene.createEntity("Editor Camera");
        auto& editorCamera = editorCameraEntity.emplace<Camera>();
        editorCamera.skyboxMaterial = skyboxMaterialAsset;
        editorCamera.renderTarget = Engine::getSystem<GraphicsSystem>()->createRenderTarget(800, 600);
        auto& editorCameraTransform = editorCameraEntity.get<Transform>();
        editorCameraTransform.setPosition({0, 0, 0});
        editorCameraEntity.emplace<EditorCamera>();


        // Light entity
        ECS::Entity lightEntity = scene.createEntity("Light");
        auto& lightTransform = lightEntity.get<Transform>();
        lightTransform.setPosition({8, 10, 8});

        lightEntity.emplace<PointLight>();
        auto& pointLight = lightEntity.get<PointLight>();
        pointLight.power = 10.0f;
        pointLight.color = glm::vec3(1.0f, 1.0f, 1.0f);

        // const AssetRef<AudioClip> music = assetManager.loadAsset<AudioClip>("music.mp3");
        // ECS::Entity speakerEntity = scene.createEntity("Speaker");
        // auto& audioSource = speakerEntity.emplace<AudioSource>();
        // audioSource.clip = music;
        // audioSource.loop = true;
        // audioSource.play();

        registerComponent<MeshRenderer>("Mesh Renderer");
        registerComponent<PointLight>("Point Light");
        registerComponent<Camera>("Camera");


        auto* imGuiSystem = Engine::getSystem<ImGuiSystem>();

        imGuiSystem->shouldDrawDockSpace = true;
        imGuiSystem->shouldDrawConsole = true;
        imGuiSystem->shouldDrawStats = true;

        editorWindows.push_back(makeBox<EditorViewportWindow>());
        editorWindows.push_back(makeBox<AssetsWindow>());
        editorWindows.push_back(makeBox<PropertiesWindow>());
        editorWindows.push_back(makeBox<SceneGraphWindow>());

        for(const auto& window : editorWindows)
        {
            window->startup();
        }

        imGuiSystem->addRenderCallback([this]()
        {
            drawMenuBar();
            for(const auto& window : editorWindows)
            {
                window->render();
            }
        });

        model->name = "monkey number 12";
    }

    void EditorSystem::update()
    {
        for(const auto& window : editorWindows)
        {
            window->update();
        }
    }

    void EditorSystem::event(Event* event)
    {
        for(const auto& window : editorWindows)
        {
            window->event(event);
        }
    }

    std::vector<ComponentInfo> EditorSystem::getComponents()
    {
        return registeredComponents;
    }

    void EditorSystem::drawMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) { /* Do something */ }
                if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Do something */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do something */ }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Windows"))
            {
                if (ImGui::MenuItem("Settings")) { /* Do something */ }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}
