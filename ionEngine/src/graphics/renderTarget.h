#pragma once
#include "device.h"
#include "textureView.h"

namespace ion
{
class RenderTarget
{
public:
    bool resize(uint32_t w, uint32_t h);

    grl::Rc<urhi::TextureView> getColorAttachment() { return m_colorAttachment; }
    grl::Rc<urhi::TextureView> getDepthAttachment() { return m_depthAttachment; }
    [[nodiscard]] uint32_t getWidth() const { return m_width; }
    [[nodiscard]] uint32_t getHeight() const { return m_height; }
private:
    friend class GraphicsSystem;
    RenderTarget(const grl::Rc<urhi::Device> &device, uint32_t w, uint32_t h, bool useDepth);

    grl::Rc<urhi::Device> m_device;

    grl::Rc<urhi::TextureView> m_colorAttachment;
    grl::Rc<urhi::TextureView> m_depthAttachment;

    bool m_useDepth;
    uint32_t m_width{};
    uint32_t m_height{};
};
}
