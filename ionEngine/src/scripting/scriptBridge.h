#pragma once
#include "Coral/Array.hpp"
#include "Coral/String.hpp"

namespace ion
{
    struct Tag;
    class Scene;
}

namespace entis
{
    class TypeErasedRegistry;
    class ViewBase;
    class Registry;
}

namespace ion::Scripting
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
            bool isKeyHeld(int key);
            bool isKeyPressed(int key);
            bool isKeyReleased(int key);
        }

        namespace EntitySystem
        {
            void registerComponentType(entis::TypeErasedRegistry* reg, uint64_t typeHash, uint64_t size, uint64_t alignment);
            size_t createEntity(entis::TypeErasedRegistry* reg);
            void* addComponent(entis::TypeErasedRegistry* reg, size_t entity, size_t componentTypeHash, void* componentData);

            entis::ViewBase* createView(entis::TypeErasedRegistry* reg, Coral::Array<uint64_t> typeHashes);
            size_t getViewSize(entis::ViewBase* view);
            Interop::EntityComponentData getViewEntry(entis::ViewBase* view, size_t index);

            size_t getTypeHash(Coral::String typeName);
        }

        namespace SceneAccess
        {
            entis::TypeErasedRegistry* getSceneRegistry();
        }

        // Component-specific accessors
        namespace ComponentAccessors
        {
            Coral::String Tag_getName(Tag* component);
            void Tag_setName(Tag* component, Coral::String);
        }

        namespace Log
        {
            void logInfo(Coral::String message);
        }
    }
}