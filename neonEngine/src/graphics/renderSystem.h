#pragma once

#include <neonRHI/neonRHI.h>

#include "graphicsSystem.h"
#include "components/camera.h"
#include "core/system.h"
#include "components/meshRenderer.h"

namespace Neon
{
    class RenderSystem final : public System
    {
    public:
        void preStartup() override;
        void render() override;
        void event(Event *event) override;
    private:
        GraphicsSystem* m_graphicsSystem{};

        Rc<RHI::Device> m_device{};
        Rc<RHI::Window> m_window{};

        Rc<RHI::Buffer> m_cameraUniformBuffer{};
        Rc<RHI::Buffer> m_pointLightsUniformBuffer{};

        Rc<RHI::TextureView> m_colorTexture{};
        Rc<RHI::TextureView> m_depthTexture{};

        Mesh m_screenMesh;
    };
}
