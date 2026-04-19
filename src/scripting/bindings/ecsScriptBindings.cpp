#include "ecsScriptBindings.h"

#include "Coral/Array.hpp"
#include "core/engine.h"
#include "core/sceneManager.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/components/meshRenderer.h"

namespace ion
{
    struct EntityComponentData
    {
        size_t entityId;
        void* const* componentPtrs;
    };

    void registerComponentType(entis::TypeErasedRegistry* reg, uint64_t typeHash, uint64_t size, uint64_t alignment)
    {
        reg->registerType(typeHash, size, alignment);
    }

    entis::ViewBase* createView(entis::TypeErasedRegistry* reg, Coral::Array<uint64_t> typeHashes)
    {
        clogr::ensure(reg != nullptr, "registry is nullptr");

        const std::vector typeVec(typeHashes.Data(), typeHashes.Data() + typeHashes.Length());
        return &reg->view(typeVec);
    }

    size_t getTypeHash(const Coral::String typeName)
    {
        const std::string name = typeName;

        clogr::info("Getting type hash for: {}", name);

        if (name == "IonEngine.Transform")
            return typeid(Transform).hash_code();
        if (name == "IonEngine.Camera")
            return typeid(Camera).hash_code();
        if (name == "IonEngine.MeshRenderer")
            return typeid(MeshRenderer).hash_code();
        if (name == "IonEngine.Tag")
            return typeid(Tag).hash_code();

        clogr::error("Unknown component type: {}", name);
        return 0;
    }

    size_t createEntity(entis::TypeErasedRegistry* reg)
    {
        return reg->getRegistry().createEntity().id();
    }

    void* addComponent(entis::TypeErasedRegistry* reg, const size_t entityId, size_t componentTypeHash, void* componentData)
    {
        entis::Entity entity = reg->getRegistry().getEntity(entityId);
        return reg->emplace(entity, componentTypeHash, componentData);
    }

    size_t getViewSize(entis::ViewBase* view)
    {
        auto* typedView = dynamic_cast<entis::TypeErasedView*>(view);
        return typedView ? typedView->size() : 0;
    }

    EntityComponentData getViewEntry(entis::ViewBase* view, size_t index)
    {
        if (!view)
        {
            clogr::error("View pointer is null");
            return EntityComponentData{0, nullptr};
        }

        const auto* typedView = dynamic_cast<entis::TypeErasedView*>(view);
        if (!typedView)
        {
            clogr::error("Failed to cast view to TypeErasedView");
            return EntityComponentData{0, nullptr};
        }

        if (index >= typedView->size())
        {
            clogr::error("Index {} out of bounds for view of size {}", index, typedView->size());
            return EntityComponentData{0, nullptr};
        }

        const entis::TypeErasedView::ComponentPack entry = typedView->at(index);

        return EntityComponentData{
            .entityId = entry.entityId,
            .componentPtrs = entry.components.empty() ? nullptr : entry.components.data()
        };
    }

    entis::TypeErasedRegistry* getSceneRegistry()
    {
        return &Engine::sceneManager().getCurrentScene().registry().asTypeErased();
    }

    Coral::String Tag_getName(const Tag* component)
    {
        return Coral::String::New(component->name);
    }

    void Tag_setName(Tag* component, const Coral::String name)
    {
        clogr::info("Tag set: {}", std::string(name));
        component->name = name;
    }

    void EcsScriptBindings::registerCalls(CallBinder &binder)
    {
        binder.bind<&createView>("IonEngine.Scene", "createViewCall");
        binder.bind<&registerComponentType>("IonEngine.Scene", "registerTypeCall");
        binder.bind<&getTypeHash>("IonEngine.Scene", "getTypeHash");

        binder.bind<&getSceneRegistry>("IonEngine.SceneManager", "getCurrentRegistryCall");

        binder.bind<&Tag_getName>("IonEngine.Tag", "getNameCall");
        binder.bind<&Tag_setName>("IonEngine.Tag", "setNameCall");


        binder.bind<&getViewSize>("IonEngine.ViewInterface", "getSizeCall");
        binder.bind<&getViewEntry>("IonEngine.ViewInterface", "getAtIndexCall");
    }
}
