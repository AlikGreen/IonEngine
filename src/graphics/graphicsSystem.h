#pragma once
#include "pipelineRegistry.h"
#include "renderTarget.h"
#include "window.h"
#include "core/system.h"
#include "shaders/shaderRegistry.h"

namespace ion
{
class GraphicsSystem final : public System
{
public:
    explicit GraphicsSystem(const WindowDesc &windowOptions);

    void preStartup() override;
    void postStartup() override;
    void preUpdate() override;
    void preRender() override;
    void postRender() override;
    void shutdown() override;

    void drawTexture(const dg::Ref<dg::ITextureView>& texture, dg::FILTER_TYPE filter = dg::FILTER_TYPE_LINEAR) const;

    [[nodiscard]] grl::Rc<RenderTarget> createRenderTarget(uint32_t width, uint32_t height, bool useDepth = true) const;

    [[nodiscard]] dg::Ref<dg::ITextureView> defaultTexture() { return m_defaultTexView; }
    [[nodiscard]] dg::Ref<dg::IRenderDevice> device() const { return m_device; }
    [[nodiscard]] dg::Ref<dg::IDeviceContext> imContext() const { return m_immediateContext; }

    [[nodiscard]] ShaderRegistry& shaderRegistry() const { return *m_shaderRegistry; }
    [[nodiscard]] PipelineRegistry& pipelineRegistry() const { return *m_pipelineRegistry; }

    [[nodiscard]] grl::Rc<Window> window() const { return m_window; }
    [[nodiscard]] dg::Ref<dg::ISwapChain> swapchain() const { return m_swapChain; }

    [[nodiscard]] float frameDuration() const { return m_frameDuration; }
private:
    WindowDesc m_windowDesc;
    grl::Rc<Window> m_window;

    grl::Box<ShaderRegistry> m_shaderRegistry{};
    grl::Box<PipelineRegistry> m_pipelineRegistry{};

    dg::Ref<dg::IRenderDevice>  m_device;
    dg::Ref<dg::IDeviceContext> m_immediateContext;
    dg::Ref<dg::ISwapChain>     m_swapChain;

    dg::Ref<dg::ITextureView> m_defaultTexView{};

    dg::ITextureView* m_renderView{};
    dg::ITextureView* m_depthView{};

    std::chrono::time_point<std::chrono::high_resolution_clock> m_frameStartTime;
    float m_frameDuration{};
};
}
