#include "meshScriptBindings.h"

#include "Coral/Array.hpp"
#include "graphics/mesh.h"
#include "scripting/handleRegistry.h"

namespace ion
{
    Coral::Array<Vertex> getVertices(const Mesh* mesh)
    {
        return Coral::Array<Vertex>::New(mesh->vertices());
    }

    void setVertices(Mesh* mesh, Coral::Array<Vertex> vertices)
    {
        std::vector<Vertex> vertVec;
        vertVec.resize(vertices.Length());

        std::memcpy(vertVec.data(), vertices.Data(), vertices.Length()*sizeof(Vertex));
        mesh->vertices(vertVec);
    }

    Coral::Array<uint32_t> getIndices(const Mesh* mesh)
    {
        return Coral::Array<uint32_t>::New(mesh->indices());
    }

    void setIndices(Mesh* mesh, Coral::Array<uint32_t> indices)
    {
        std::vector<uint32_t> indexVec;
        indexVec.resize(indices.Length());

        std::memcpy(indexVec.data(), indices.Data(), indices.Length()*sizeof(uint32_t));
        mesh->indices(indexVec);
    }

    uint32_t createMesh()
    {
        return HandleRegistry<Mesh>::instance().registerHandle(grl::makeRc<Mesh>());
    }

    Mesh* getMesh(const uint32_t handle)
    {
        return HandleRegistry<Mesh>::instance().get(handle);
    }

    void releaseMesh(const uint32_t handle)
    {
        HandleRegistry<Mesh>::instance().release(handle);
    }

    void MeshScriptBindings::registerCalls(CallBinder &binder)
    {
        binder.bind<getVertices>("IonEngine.NativeBridge", "Mesh_getVertices");
        binder.bind<setVertices>("IonEngine.NativeBridge", "Mesh_setVertices");
        binder.bind<getIndices>("IonEngine.NativeBridge", "Mesh_getIndices");
        binder.bind<setIndices>("IonEngine.NativeBridge", "Mesh_setIndices");

        binder.bind<createMesh>("IonEngine.NativeBridge", "Mesh_create");
        binder.bind<getMesh>("IonEngine.NativeBridge", "Mesh_get");
        binder.bind<releaseMesh>("IonEngine.NativeBridge", "Mesh_release");
    }
}
