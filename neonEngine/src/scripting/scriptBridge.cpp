#include "scriptBridge.h"

#include <neonECS/neonECS.h>

#include "log.h"
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

namespace Neon::Scripting::ExportedAPI
{
    namespace Input
    {
        extern "C"
        {
            bool isKeyHeld(int key)
            {
                return Neon::Input::isKeyHeld(static_cast<KeyCode>(key));
            }

            bool isKeyPressed(int key)
            {
                return Neon::Input::isKeyPressed(static_cast<KeyCode>(key));
            }

            bool isKeyReleased(int key)
            {
                return Neon::Input::isKeyReleased(static_cast<KeyCode>(key));
            }
        }
    }

    namespace EntitySystem
    {
        extern "C"
        {
            void registerComponentType(ECS::TypeErasedRegistry* reg, uint64_t typeHash, uint64_t size, uint64_t alignment)
            {
                reg->registerType(typeHash, size, alignment);
            }

            size_t createEntity(ECS::TypeErasedRegistry* reg)
            {
                return reg->getRegistry().createEntity().id();
            }

            void* addComponent(ECS::TypeErasedRegistry* reg, const size_t entityId, size_t componentTypeHash, void* componentData)
            {
                ECS::Entity entity = reg->getRegistry().getEntity(entityId);
                return reg->emplace(entity, componentTypeHash, componentData);
            }

            ECS::ViewBase* createView(ECS::TypeErasedRegistry* reg, Coral::Array<uint64_t> typeHashes)
            {
                Debug::ensure(reg != nullptr, "registry is nullptr");

                const std::vector typeVec(typeHashes.Data(), typeHashes.Data() + typeHashes.Length());
                return &reg->view(typeVec);
            }

            size_t getViewSize(ECS::ViewBase* view)
            {
                auto* typedView = dynamic_cast<ECS::TypeErasedView*>(view);
                return typedView ? typedView->size() : 0;
            }

            Interop::EntityComponentData getViewEntry(ECS::ViewBase* view, size_t index)
            {
                if (!view)
                {
                    Neon::Log::error("View pointer is null");
                    return Interop::EntityComponentData{0, nullptr};
                }

                auto* typedView = dynamic_cast<ECS::TypeErasedView*>(view);
                if (!typedView)
                {
                    Neon::Log::error("Failed to cast view to TypeErasedView");
                    return Interop::EntityComponentData{0, nullptr};
                }

                if (index >= typedView->size())
                {
                    Neon::Log::error("Index {} out of bounds for view of size {}", index, typedView->size());
                    return Interop::EntityComponentData{0, nullptr};
                }

                ECS::TypeErasedView::ComponentPack entry = typedView->at(index);

                return Interop::EntityComponentData{
                    .entityId = entry.entityId,
                    .componentPtrs = entry.components.empty() ? nullptr : entry.components.data()
                };
            }

            size_t getTypeHash(Coral::String typeName)
            {
                const std::string name = typeName;

                Neon::Log::info("Getting type hash for: {}", name);

                if (name == "NeonEngine.Transform")
                    return typeid(Transform).hash_code();
                if (name == "NeonEngine.Camera")
                    return typeid(Camera).hash_code();
                if (name == "NeonEngine.MeshRenderer")
                    return typeid(MeshRenderer).hash_code();
                if (name == "NeonEngine.Tag")
                    return typeid(Tag).hash_code();

                Neon::Log::error("Unknown component type: {}", name);
                return 0;
            }
        }
    }

    namespace SceneAccess
    {
        extern "C"
        {
            ECS::TypeErasedRegistry* getSceneRegistry()
            {
                return &Engine::getSceneManager().getCurrentScene().getRegistry().asTypeErased();
            }
        }
    }

    namespace ComponentAccessors
    {
        extern "C"
        {
            Coral::String Tag_getName(Tag* component)
            {
                return Coral::String::New(component->name);
            }

            void Tag_setName(Tag* component, Coral::String name)
            {
                Neon::Log::info("Tag set: {}", std::string(name));
                component->name = name;
            }
        }
    }

    namespace Log
    {
        extern "C"
        {
            void logInfo(Coral::String message)
            {
                Neon::Log::info("[C#] {}", static_cast<std::string>(message));
            }
        }
    }
}
