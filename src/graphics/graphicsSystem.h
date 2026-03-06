#pragma once
#include "context.h"
#include "device.h"
#include "renderTarget.h"
#include "swapchain.h"
#include "core/system.h"
#include "descriptions/windowDesc.h"

namespace ion
{
class GraphicsSystem final : public System
{
public:
    explicit GraphicsSystem(const urhi::WindowDesc &windowOptions);

    void preStartup() override;
    void preUpdate() override;
    void preRender() override;
    void postRender() override;
    void shutdown() override;

    void drawTexture(const grl::Rc<urhi::Texture> &texture, urhi::TextureFilter filter = urhi::TextureFilter::Linear) const;

    [[nodiscard]] grl::Rc<RenderTarget> createRenderTarget(uint32_t width, uint32_t height, bool useDepth = true) const;

    [[nodiscard]] grl::Rc<urhi::TextureView> getDefaultTexture() { return m_defaultTexture; }
    [[nodiscard]] grl::Rc<urhi::Device> getDevice() const { return m_device; }
    [[nodiscard]] grl::Rc<urhi::Window> getWindow() const { return m_window; }
    [[nodiscard]] grl::Rc<urhi::Swapchain> getSwapchain() const { return m_swapchain; }
private:
    urhi::WindowDesc m_windowDesc;

    grl::Rc<urhi::Context> m_context;
    grl::Rc<urhi::Device> m_device;
    grl::Rc<urhi::Window> m_window;
    grl::Rc<urhi::Swapchain> m_swapchain{};

    grl::Rc<urhi::TextureView> m_renderView{};
    grl::Rc<urhi::TextureView> m_defaultTexture{};
};
}
