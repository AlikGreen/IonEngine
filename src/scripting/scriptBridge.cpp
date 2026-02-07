#include "scriptBridge.h"

#include <entis/entis.h>

#include <clogr.h>
#include "Coral/Array.hpp"
#include "Coral/String.hpp"
#include "core/scene.h"
#include "core/engine.h"
#include "core/sceneManager.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "graphics/components/camera.h"
#include "graphics/components/meshRenderer.h"
#include "input/input.h"

namespace ion::Scripting::ExportedAPI
{
    namespace Input
    {
        bool isKeyHeld(int key)
        {
            return ion::Input::isKeyHeld(static_cast<KeyCode>(key));
        }

        bool isKeyPressed(int key)
        {
            return ion::Input::isKeyPressed(static_cast<KeyCode>(key));
        }

        bool isKeyReleased(int key)
        {
            return ion::Input::isKeyReleased(static_cast<KeyCode>(key));
        }
    }

    namespace EntitySystem
    {
        void registerComponentType(entis::TypeErasedRegistry* reg, uint64_t typeHash, uint64_t size, uint64_t alignment)
        {
            reg->registerType(typeHash, size, alignment);
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

        entis::ViewBase* createView(entis::TypeErasedRegistry* reg, Coral::Array<uint64_t> typeHashes)
        {
            clogr::ensure(reg != nullptr, "registry is nullptr");

            const std::vector typeVec(typeHashes.Data(), typeHashes.Data() + typeHashes.Length());
            return &reg->view(typeVec);
        }

        size_t getViewSize(entis::ViewBase* view)
        {
            auto* typedView = dynamic_cast<entis::TypeErasedView*>(view);
            return typedView ? typedView->size() : 0;
        }

        Interop::EntityComponentData getViewEntry(entis::ViewBase* view, size_t index)
        {
            if (!view)
            {
                clogr::error("View pointer is null");
                return Interop::EntityComponentData{0, nullptr};
            }

            auto* typedView = dynamic_cast<entis::TypeErasedView*>(view);
            if (!typedView)
            {
                clogr::error("Failed to cast view to TypeErasedView");
                return Interop::EntityComponentData{0, nullptr};
            }

            if (index >= typedView->size())
            {
                clogr::error("Index {} out of bounds for view of size {}", index, typedView->size());
                return Interop::EntityComponentData{0, nullptr};
            }

            entis::TypeErasedView::ComponentPack entry = typedView->at(index);

            return Interop::EntityComponentData{
                .entityId = entry.entityId,
                .componentPtrs = entry.components.empty() ? nullptr : entry.components.data()
            };
        }

        size_t getTypeHash(Coral::String typeName)
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
    }

    namespace SceneAccess
    {
        entis::TypeErasedRegistry* getSceneRegistry()
        {
            return &Engine::getSceneManager().getCurrentScene().getRegistry().asTypeErased();
        }
    }

    namespace ComponentAccessors
    {
            Coral::String Tag_getName(Tag* component)
            {
                return Coral::String::New(component->name);
            }

            void Tag_setName(Tag* component, Coral::String name)
            {
                clogr::info("Tag set: {}", std::string(name));
                component->name = name;
            }
    }

    namespace Log
    {
        void logInfo(Coral::String message)
        {
            clogr::info("[C#] {}", static_cast<std::string>(message));
        }
    }
}
