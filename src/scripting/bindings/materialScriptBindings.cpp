#include "materialScriptBindings.h"

#include "glm/glm.hpp"
#include "graphics/materialInstance.h"
#include "scripting/handleRegistry.h"

namespace ion
{
    uint32_t MaterialInstance_create()
    {
        auto material = grl::makeRc<MaterialInstance>(MaterialTemplates::pbr());
        material->set("baseColor", glm::vec4(1.0f));
        return HandleRegistry<MaterialInstance>::instance().registerHandle(material);
    }

    MaterialInstance* MaterialInstance_get(const uint32_t handle)
    {
        return HandleRegistry<MaterialInstance>::instance().get(handle);
    }

    void MaterialInstance_release(const uint32_t handle)
    {
        HandleRegistry<MaterialInstance>::instance().release(handle);
    }

    void MaterialScriptBindings::registerCalls(CallBinder &binder)
    {
        binder.bind<MaterialInstance_create>("IonEngine.NativeBridge", "MaterialInstance_create");
        binder.bind<MaterialInstance_get>("IonEngine.NativeBridge", "MaterialInstance_get");
        binder.bind<MaterialInstance_release>("IonEngine.NativeBridge", "MaterialInstance_release");
    }
}
