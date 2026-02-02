#include "graphicsSystem.h"

#include "shaderCompiler.h"
#include "window.h"
#include "core/engine.h"
#include "core/eventManager.h"
#include "events/dropFileEvent.h"
#include "events/quitEvent.h"
#include "events/rhiWindowEvent.h"
#include "events/windowResizeEvent.h"
#include "implementations/opengl/blitShader.h"
#include "input/events/keyDownEvent.h"
#include "input/events/keyUpEvent.h"
#include "input/events/mouseButtonDownEvent.h"
#include "input/events/mouseButtonUpEvent.h"
#include "input/events/mouseMoveEvent.h"
#include "input/events/mouseWheelEvent.h"
#include "input/events/textInputEvent.h"

namespace Neon
{
    GraphicsSystem::GraphicsSystem(const RHI::WindowCreationOptions &windowOptions)
    {
        m_window = RHI::Window::createWindow(windowOptions);
    }

    void GraphicsSystem::updateSwapchainFramebuffers()
    {
        std::vector<Rc<RHI::Texture>> textures = m_swapchain->getTextures();
        m_renderTextures.clear();

        for(const auto& texture : textures)
        {
            const RHI::TextureViewDesc viewDesc(texture);
            Rc<RHI::TextureView> colView = m_device->createTextureView(viewDesc);
            m_renderTextures.push_back(colView);
        }
    }

    void GraphicsSystem::preStartup()
    {
        m_window->run();
        m_device = m_window->createDevice();

        RHI::SwapchainDesc swapchainDesc{};
        swapchainDesc.window = m_window;
        m_swapchain = m_device->createSwapchain(swapchainDesc);

        const std::vector<glm::vec2> quadPositions =
        {
            {-1, -1 },
            { 1, -1 },
            { 1,  1 },
            {-1,  1 }
        };

        const std::vector<uint32_t> quadIndices =
        {
            0, 1, 2,
            0, 2, 3
        };

        RHI::ShaderCompileDescription compileDesc{};
        compileDesc.path = "graphicsSystemBlitShader.slang";
        compileDesc.source = RHI::blitShaderSource;
        auto spirv = RHI::ShaderCompiler::compile(compileDesc);

        Rc<RHI::Shader> shader = m_device->createShader(spirv);
        shader->compile();

        RHI::InputLayout vertexInputState{};
        vertexInputState.addVertexBuffer<glm::vec2>(0);
        vertexInputState.addVertexAttribute<glm::vec2>(0, 0);

        RHI::DepthState depthState{};
        depthState.hasDepthTarget  = false;
        depthState.enableDepthTest = false;

        RHI::RasterizerState rasterizerState{};
        rasterizerState.cullMode = RHI::CullMode::None;

        RHI::BlendState blendState{};
        blendState.enableBlend = true;

        RHI::GraphicsPipelineDesc pipelineDescription{};
        pipelineDescription.shader             = shader;
        pipelineDescription.inputLayout		   = vertexInputState;
        pipelineDescription.targetsDescription = {};
        pipelineDescription.depthState         = depthState;
        pipelineDescription.rasterizerState    = rasterizerState;
        pipelineDescription.blendState         = blendState;

        m_pipeline = m_device->createPipeline(pipelineDescription);

        updateSwapchainFramebuffers();

        m_vertexBuffer = m_device->createVertexBuffer();
        m_indexBuffer = m_device->createIndexBuffer();

        constexpr uint32_t texSize = 1;
        RHI::TextureDesc desc = RHI::TextureDesc::Texture2D(
            texSize,
            texSize,
            RHI::PixelFormat::R8G8B8A8Unorm,
            RHI::TextureUsage::Sampled
        );

        Rc<RHI::Texture> texture = m_device->createTexture(desc);

        uint8_t pixel[4] = {255, 255, 255, 255};

        Rc<RHI::CommandList> cl = m_device->createCommandList();

        RHI::TextureUploadDesc uploadDesc{};
        uploadDesc.width = texSize;
        uploadDesc.height = texSize;
        uploadDesc.data = pixel;

        cl->begin();

        cl->updateTexture(texture, uploadDesc);

        cl->reserveBuffer(m_vertexBuffer, quadPositions.size() * sizeof(glm::vec2));
        cl->updateBuffer(m_vertexBuffer, quadPositions);

        cl->reserveBuffer(m_indexBuffer, quadIndices.size() * sizeof(uint32_t));
        cl->updateBuffer(m_indexBuffer, quadIndices);

        m_device->submit(cl);

        const auto viewDesc = RHI::TextureViewDesc(texture);
        m_defaultTexture = m_device->createTextureView(viewDesc);
    }

    void GraphicsSystem::preUpdate()
    {
        std::vector<RHI::Event> events = m_window->pollEvents();

        EventManager& eventManager = Engine::getEventManager();

        for(const auto& event : events)
        {
            eventManager.queueEvent<RhiWindowEvent>(event);

            switch (event.type)
            {
                case RHI::Event::Type::Quit:
                {
                    eventManager.queueEvent<QuitEvent>();
                    break;
                }
                case RHI::Event::Type::WindowResize:
                {
                    const auto& windowEvent = std::get<RHI::Event::WindowResizeEvent>(event.data);
                    eventManager.queueEvent<WindowResizeEvent>(windowEvent.width, windowEvent.height);
                    m_swapchain->resize(windowEvent.width, windowEvent.height);
                    updateSwapchainFramebuffers();
                    break;
                }
                case RHI::Event::Type::KeyDown:
                {
                    const auto& keyEvent = std::get<RHI::Event::KeyEvent>(event.data);
                    eventManager.queueEvent<KeyDownEvent>(keyEvent.key, keyEvent.repeat);
                    break;
                }
                case RHI::Event::Type::KeyUp:
                {
                    const auto& keyEvent = std::get<RHI::Event::KeyEvent>(event.data);
                    eventManager.queueEvent<KeyUpEvent>(keyEvent.key);
                    break;
                }
                case RHI::Event::Type::MouseButtonDown:
                {
                    const auto& buttonEvent = std::get<RHI::Event::MouseButtonEvent>(event.data);
                    eventManager.queueEvent<MouseButtonDownEvent>(buttonEvent.button);
                    break;
                }
                case RHI::Event::Type::MouseButtonUp:
                {
                    const auto& buttonEvent = std::get<RHI::Event::MouseButtonEvent>(event.data);
                    eventManager.queueEvent<MouseButtonUpEvent>(buttonEvent.button);
                    break;
                }
                case RHI::Event::Type::MouseMotion:
                {
                    const auto& motionEvent = std::get<RHI::Event::MouseMotionEvent>(event.data);
                    eventManager.queueEvent<MouseMoveEvent>(motionEvent.x, motionEvent.y);
                    break;
                }
                case RHI::Event::Type::MouseWheel:
                {
                    const auto& wheelEvent = std::get<RHI::Event::MouseWheelEvent>(event.data);
                    eventManager.queueEvent<MouseWheelEvent>(wheelEvent.x, wheelEvent.y);
                    break;
                }
                case RHI::Event::Type::TextInput:
                {
                    const auto& textEvent = std::get<RHI::Event::TextInputEvent>(event.data);
                    eventManager.queueEvent<TextInputEvent>(textEvent.codepoint);
                    break;
                }
                case RHI::Event::Type::DropFile:
                {
                    const auto& dropEvent = std::get<RHI::Event::DropFileEvent>(event.data);
                    eventManager.queueEvent<DropFileEvent>(dropEvent.path);
                    break;
                }
            }
        }
    }

    void GraphicsSystem::preRender()
    {
        m_imageIndex = m_swapchain->acquireNextImage();
    }

    void GraphicsSystem::postRender()
    {
        m_swapchain->present(m_imageIndex);
    }

    void GraphicsSystem::shutdown()
    {
        m_window->close();
    }

    void GraphicsSystem::drawTexture(const Rc<RHI::TextureView> &texture, const Rc<RHI::Sampler> &sampler) const
    {
        const Rc<RHI::CommandList> cmd = m_device->createCommandList();

        cmd->begin();


        RHI::ColorAttachment colorAttachment{};
        colorAttachment.texture = m_renderTextures[m_imageIndex];

        RHI::RenderPassDesc renderPassDesc{};
        renderPassDesc.colorAttachments = {colorAttachment};
        cmd->beginRenderPass(renderPassDesc);

        cmd->setPipeline(m_pipeline);

        cmd->setIndexBuffer(m_indexBuffer, RHI::IndexFormat::UInt32);
        cmd->setVertexBuffer(0, m_vertexBuffer);

        cmd->setTexture("blitTexture", texture);
        cmd->setSampler("blitSampler", sampler);

        cmd->drawIndexed(6);

        cmd->endRenderPass();

        m_device->submit(cmd);
    }

    Rc<RenderTarget> GraphicsSystem::createRenderTarget(const uint32_t width, const uint32_t height, const bool useDepth) const
    {
        return Rc<RenderTarget>(new RenderTarget(m_device, width, height, useDepth));
    }
}
