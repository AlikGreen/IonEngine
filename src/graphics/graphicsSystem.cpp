#include "graphicsSystem.h"

#include "EngineFactoryVk.h"
#include "window.h"
#include "core/engine.h"
#include "core/eventManager.h"
#include "events/dropFileEvent.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "helpers/gfx.h"


namespace ion
{
    GraphicsSystem::GraphicsSystem(const WindowDesc &windowOptions)
        : m_windowDesc(windowOptions) { }

    void GraphicsSystem::preStartup()
    {
        m_window = grl::makeBox<Window>(m_windowDesc);

        dg::EngineVkCreateInfo engineCI{};

        dg::IEngineFactoryVk* pFactoryVk = dg::LoadAndGetEngineFactoryVk();
        pFactoryVk->CreateDeviceAndContextsVk(engineCI, &m_device, &m_immediateContext);

        HWND hwnd = glfwGetWin32Window(m_window->handle());

        dg::SwapChainDesc swapChainDesc;

        const dg::Win32NativeWindow window{hwnd};
        pFactoryVk->CreateSwapChainVk(m_device, m_immediateContext, swapChainDesc, window, &m_swapChain);

        dg::SetDebugMessageCallback(
            [](dg::DEBUG_MESSAGE_SEVERITY severity, const Diligent::Char* message,
               const Diligent::Char* function, const Diligent::Char* file, int line)
            {
                if (severity == dg::DEBUG_MESSAGE_SEVERITY_ERROR)
                {
                    clogr::abort("{}", message);
                }
            }
        );


        dg::TextureDesc texDesc{};
        texDesc.Format = dg::TEX_FORMAT_RGBA8_UNORM;
        texDesc.Width = 1;
        texDesc.Height = 1;
        texDesc.BindFlags = dg::BIND_SHADER_RESOURCE;
        texDesc.Usage     = dg::USAGE_IMMUTABLE;
        texDesc.Type = dg::RESOURCE_DIM_TEX_2D;

        auto data = new uint8_t[]{ 255, 255, 255, 255 };

        dg::TextureSubResData subResData;
        subResData.pData  = data;
        subResData.Stride = 4;

        dg::TextureData texData;
        texData.pSubResources   = &subResData;
        texData.NumSubresources = 1;

        dg::Ref<dg::ITexture> defaultTex;
        m_device->CreateTexture(texDesc, &texData, &defaultTex);

        m_defaultTexView = defaultTex->GetDefaultView(dg::TEXTURE_VIEW_SHADER_RESOURCE);

        m_shaderRegistry = grl::makeBox<ShaderRegistry>(m_device);
        m_pipelineRegistry = grl::makeBox<PipelineRegistry>(m_device);

        gfx::init(*this);
    }

    void GraphicsSystem::postStartup()
    {
        m_window->show();
    }

    void GraphicsSystem::preUpdate()
    {
        m_window->pollEvents();
    }

    void GraphicsSystem::preRender()
    {
        m_renderView = m_swapChain->GetCurrentBackBufferRTV();
        m_depthView = m_swapChain->GetDepthBufferDSV();
    }

    void GraphicsSystem::postRender()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto frameTime = std::chrono::duration<float>(endTime - m_frameStartTime);
        m_frameDuration = frameTime.count();
        m_swapChain->Present();
        m_frameStartTime = std::chrono::high_resolution_clock::now();
    }

    void GraphicsSystem::shutdown()
    {
        m_window->close();
    }

    void GraphicsSystem::drawTexture(const dg::Ref<dg::ITextureView>& texture, dg::FILTER_TYPE filter) const
    {
        clogr::abort("Not implemented");
        // TODO make a blit texture pipeline

        // cmd->begin();
        //
        // urhi::BlitTextureDesc blitDesc;
        // blitDesc.src = texture;
        // blitDesc.dst = m_renderView->texture();
        // blitDesc.filter = filter;
        //
        // cmd->blitTexture(blitDesc);
        //
        // m_device->submit(cmd);
    }

    grl::Rc<RenderTarget> GraphicsSystem::createRenderTarget(const uint32_t width, const uint32_t height, const bool useDepth) const
    {
        return grl::Rc<RenderTarget>(new RenderTarget(m_device, width, height, useDepth));
    }
}
