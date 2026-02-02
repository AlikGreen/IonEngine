#pragma once
#include "Coral/Array.hpp"
#include "Coral/String.hpp"

namespace Neon
{
    struct Tag;
    class Scene;
}

namespace Neon::ECS
{
    class TypeErasedRegistry;
    class ViewBase;
    class Registry;
}

namespace Neon::Scripting
{
    // Data structures for marshaling between C++ and scripts
    namespace Interop
    {
        struct EntityComponentData
        {
            size_t entityId;
            void* const* componentPtrs;
        };
    }

    // C API boundary - functions exported to script runtime
    namespace ExportedAPI
    {
        namespace Input
        {
            extern "C"
            {
                bool isKeyHeld(int key);
                bool isKeyPressed(int key);
                bool isKeyReleased(int key);
            }
        }

        namespace EntitySystem
        {
            extern "C"
            {
                void registerComponentType(ECS::TypeErasedRegistry* reg, uint64_t typeHash, uint64_t size, uint64_t alignment);
                size_t createEntity(ECS::TypeErasedRegistry* reg);
                void* addComponent(ECS::TypeErasedRegistry* reg, size_t entity, size_t componentTypeHash, void* componentData);

                ECS::ViewBase* createView(ECS::TypeErasedRegistry* reg, Coral::Array<uint64_t> typeHashes);
                size_t getViewSize(ECS::ViewBase* view);
                Interop::EntityComponentData getViewEntry(ECS::ViewBase* view, size_t index);

                size_t getTypeHash(Coral::String typeName);
            }
        }

        namespace SceneAccess
        {
            extern "C"
            {
                ECS::TypeErasedRegistry* getSceneRegistry();
            }
        }

        // Component-specific accessors
        namespace ComponentAccessors
        {
            extern "C"
            {
                Coral::String Tag_getName(Tag* component);
                void Tag_setName(Tag* component, Coral::String);
            }
        }

        namespace Log
        {
            extern "C"
            {
                void logInfo(Coral::String message);
            }
        }
    }
}