#pragma once
#include "device.h"
#include "renderTarget.h"
#include "core/system.h"
#include "descriptions/windowCreationOptions.h"

namespace ion
{
class GraphicsSystem final : public System
{
public:
    explicit GraphicsSystem(const urhi::WindowCreationOptions &windowOptions);

    void preStartup() override;
    void preUpdate() override;
    void preRender() override;
    void postRender() override;
    void shutdown() override;

    void drawTexture(const grl::Rc<urhi::TextureView> &texture, const grl::Rc<urhi::Sampler> &sampler) const;

    [[nodiscard]] grl::Rc<RenderTarget> createRenderTarget(uint32_t width, uint32_t height, bool useDepth = true) const;

    [[nodiscard]] grl::Rc<urhi::TextureView> getDefaultTexture() { return m_defaultTexture; }
    [[nodiscard]] grl::Rc<urhi::Device> getDevice() const { return m_device; }
    [[nodiscard]] grl::Rc<urhi::Window> getWindow() const { return m_window; }
    [[nodiscard]] grl::Rc<urhi::Swapchain> getSwapchain() const { return m_swapchain; }
private:
    uint32_t m_imageIndex{};

    grl::Rc<urhi::Device> m_device;
    grl::Rc<urhi::Window> m_window;
    grl::Rc<urhi::Swapchain> m_swapchain{};

    grl::Rc<urhi::TextureView> m_defaultTexture{};
    grl::Rc<urhi::Buffer> m_vertexBuffer{};
    grl::Rc<urhi::Buffer> m_indexBuffer{};
    grl::Rc<urhi::Pipeline> m_pipeline{};

    std::vector<grl::Rc<urhi::TextureView>> m_renderTextures{};

    void updateSwapchainFramebuffers();
};
}
