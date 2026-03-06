#include "graphicsSystem.h"

#include "shaderCompiler.h"
#include "window.h"
#include "core/engine.h"
#include "core/eventManager.h"
#include "events/dropFileEvent.h"
#include "events/quitEvent.h"
#include "events/rhiWindowEvent.h"
#include "events/windowResizeEvent.h"
#include "input/events/keyDownEvent.h"
#include "input/events/keyUpEvent.h"
#include "input/events/mouseButtonDownEvent.h"
#include "input/events/mouseButtonUpEvent.h"
#include "input/events/mouseMoveEvent.h"
#include "input/events/mouseWheelEvent.h"
#include "input/events/textInputEvent.h"

#include "sinks/fileSink.h"

namespace ion
{
    GraphicsSystem::GraphicsSystem(const urhi::WindowDesc &windowOptions)
        : m_windowDesc(windowOptions) { }

    void GraphicsSystem::preStartup()
    {
        clogr::getDefaultLogger()->addSink<clogr::FileSink>(R"(C:\Users\alikg\Downloads\log.log)");
        m_context = urhi::Context::create(urhi::BackendAPI::Vulkan);

        m_window = m_context->createWindow(m_windowDesc);
        m_device = m_context->createDevice({ m_window });

        urhi::SwapchainDesc swapchainDesc{};
        swapchainDesc.window = m_window;
        swapchainDesc.device = m_device;

        m_swapchain = m_context->createSwapchain(swapchainDesc);

        constexpr uint32_t texSize = 1;
        const urhi::TextureDesc desc = urhi::TextureDesc::Texture2D(
            texSize,
            texSize,
            urhi::PixelFormat::RGBA8UNorm,
            urhi::TextureUsage::Sampled
        );

        const grl::Rc<urhi::Texture> texture = m_device->createTexture(desc);

        const auto viewDesc = urhi::TextureViewDesc(texture);
        m_defaultTexture = m_device->createTextureView(viewDesc);
    }

    void GraphicsSystem::preUpdate()
    {
        std::vector<urhi::Event> events = m_window->pollEvents();

        EventManager& eventManager = Engine::getEventManager();

        for(const auto& event : events)
        {
            eventManager.queueEvent<RhiWindowEvent>(event);

            switch (event.type)
            {
                case urhi::Event::Type::Quit:
                {
                    eventManager.queueEvent<QuitEvent>();
                    break;
                }
                case urhi::Event::Type::WindowResize:
                {
                    const auto& windowEvent = std::get<urhi::Event::WindowResizeEvent>(event.data);
                    eventManager.queueEvent<WindowResizeEvent>(windowEvent.width, windowEvent.height);
                    m_swapchain->resize(windowEvent.width, windowEvent.height);
                    break;
                }
                case urhi::Event::Type::KeyDown:
                {
                    const auto& keyEvent = std::get<urhi::Event::KeyEvent>(event.data);
                    eventManager.queueEvent<KeyDownEvent>(keyEvent.key, keyEvent.repeat);
                    break;
                }
                case urhi::Event::Type::KeyUp:
                {
                    const auto& keyEvent = std::get<urhi::Event::KeyEvent>(event.data);
                    eventManager.queueEvent<KeyUpEvent>(keyEvent.key);
                    break;
                }
                case urhi::Event::Type::MouseButtonDown:
                {
                    const auto& buttonEvent = std::get<urhi::Event::MouseButtonEvent>(event.data);
                    eventManager.queueEvent<MouseButtonDownEvent>(buttonEvent.button);
                    break;
                }
                case urhi::Event::Type::MouseButtonUp:
                {
                    const auto& buttonEvent = std::get<urhi::Event::MouseButtonEvent>(event.data);
                    eventManager.queueEvent<MouseButtonUpEvent>(buttonEvent.button);
                    break;
                }
                case urhi::Event::Type::MouseMotion:
                {
                    const auto& motionEvent = std::get<urhi::Event::MouseMotionEvent>(event.data);
                    eventManager.queueEvent<MouseMoveEvent>(motionEvent.x, motionEvent.y);
                    break;
                }
                case urhi::Event::Type::MouseWheel:
                {
                    const auto& wheelEvent = std::get<urhi::Event::MouseWheelEvent>(event.data);
                    eventManager.queueEvent<MouseWheelEvent>(wheelEvent.x, wheelEvent.y);
                    break;
                }
                case urhi::Event::Type::TextInput:
                {
                    const auto& textEvent = std::get<urhi::Event::TextInputEvent>(event.data);
                    eventManager.queueEvent<TextInputEvent>(textEvent.codepoint);
                    break;
                }
                case urhi::Event::Type::DropFile:
                {
                    const auto& dropEvent = std::get<urhi::Event::DropFileEvent>(event.data);
                    eventManager.queueEvent<DropFileEvent>(dropEvent.path);
                    break;
                }
            }
        }
    }

    void GraphicsSystem::preRender()
    {
        m_renderView = m_swapchain->acquireNextImage();
    }

    void GraphicsSystem::postRender()
    {
        m_swapchain->present();
    }

    void GraphicsSystem::shutdown()
    {
        m_window->close();
    }

    void GraphicsSystem::drawTexture(const grl::Rc<urhi::Texture> &texture, const urhi::TextureFilter filter) const
    {
        const grl::Rc<urhi::CommandList> cmd = m_device->acquireCommandList(urhi::QueueType::Graphics);

        cmd->begin();

        urhi::BlitTextureDesc blitDesc;
        blitDesc.src = texture;
        blitDesc.dst = m_renderView->texture();
        blitDesc.filter = filter;

        cmd->blitTexture(blitDesc);

        m_device->submit(cmd);
    }

    grl::Rc<RenderTarget> GraphicsSystem::createRenderTarget(const uint32_t width, const uint32_t height, const bool useDepth) const
    {
        return grl::Rc<RenderTarget>(new RenderTarget(m_device, width, height, useDepth));
    }
}
