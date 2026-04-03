#include "meshSerializer.h"

#include "asset/assetStream.h"

namespace ion
{
    void MeshSerializer::serialize(AssetStream &assetStream, AssetRegistry &assetRegistry, AssetDeps& deps, const Mesh& mesh)
    {
        assetStream.write(mesh.readbackIndices());
        assetStream.write(mesh.readbackVertices());
        assetStream.write(mesh.primitives());
    }

    grl::Rc<Mesh> MeshSerializer::deserialize(AssetStream &assetStream, AssetRegistry &assetRegistry)
    {
        std::vector<uint32_t> indices;
        assetStream.read(indices);

        std::vector<Vertex> vertices;
        assetStream.read(vertices);

        std::vector<Primitive> primitives;
        assetStream.read(primitives);

        const auto mesh = grl::makeRc<Mesh>();
        mesh->vertices(std::move(vertices));
        mesh->indices(std::move(indices));
        mesh->primitives(std::move(primitives));

        return mesh;
    }
}
