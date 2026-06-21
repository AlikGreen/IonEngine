#pragma once
#include <tiny_gltf.h>

#include "asset/assetImporter.h"
#include "asset/assetRef.h"
#include "core/scene.h"
#include "graphics/materialInstance.h"
#include "graphics/mesh.h"

namespace ion
{
class GLBSceneImporter final : public AssetImporter<Scene>
{
public:
    grl::Box<Scene> import(const std::filesystem::path& filePath, const NoOptions&) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
private:
    static bool loadModel(tinygltf::Model& model, const std::string& filePath);

    static std::vector<AssetRef<MaterialInstance>> extractMaterials(const tinygltf::Model& model);
    static AssetRef<MaterialInstance> processMaterial(const tinygltf::Model &model, tinygltf::Material gltfMat);

    static auto extractMeshes(tinygltf::Model &model) -> std::vector<AssetRef<Mesh> >;
    static AssetRef<Mesh> processMesh(const tinygltf::Mesh& mesh, const tinygltf::Model &model);

    static std::vector<glm::vec3> extractVertexPositions(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<glm::vec3> extractVertexNormals(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<glm::vec2> extractVertexUVs(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<uint32_t> extractIndices(const tinygltf::Primitive& primitive, const tinygltf::Model& model);

    static AssetRef<Image> loadTexture(const tinygltf::Texture& texture, const tinygltf::Model& model, bool isSrgb);

    static void processNodes(Scene &scene,
        const std::vector<AssetRef<MaterialInstance>> &materials,
        const std::vector<AssetRef<Mesh>> &meshes,
        const tinygltf::Model &model);

    static void setupTransform(entis::Entity &entity, const tinygltf::Node &node);
    static void setupMeshRenderer(
        entis::Entity &entity,
        const AssetRef<Mesh> &meshHandle,
        const tinygltf::Mesh &mesh,
        const std::vector<AssetRef<MaterialInstance>> &materials);
};
}
