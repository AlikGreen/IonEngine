#pragma once
#include <tiny_gltf.h>

#include "asset/assetImporter.h"
#include "core/scene.h"
#include "graphics/image.h"
#include "graphics/materialInstance.h"
#include "graphics/mesh.h"

namespace ion
{
    class GLBSceneImporter final : public AssetImporter<Scene>
{
public:
    grl::Box<Scene> import(const std::filesystem::path& filePath, ImportOptions<Scene> options) override;
    [[nodiscard]] bool canImport(const std::filesystem::path &src) const override;
private:
    static bool loadModel(tinygltf::Model& model, const std::string& filePath);
    static std::vector<AssetRef<MaterialInstance>> processMaterials(const tinygltf::Model& model);
    static void processNodes(const tinygltf::Model& model, Scene& scene, const std::vector<AssetRef<MaterialInstance>>& materials, const AssetRef<MaterialInstance>& defaultMaterial);

    static void setupTransform(entis::Entity& entity, const tinygltf::Node& node);
    static void setupMeshRenderer(entis::Entity& entity, const AssetRef<Mesh> &meshHandle, const tinygltf::Mesh& mesh, const AssetRef<MaterialInstance>& defaultMaterial, const std::vector<AssetRef<MaterialInstance>>& materials);
    static void setupPBRProperties(const AssetRef<MaterialInstance> &mat, const tinygltf::Material& material, const tinygltf::Model& model);
    static void setupTextureProperties(const AssetRef<MaterialInstance> &mat, const tinygltf::Material& material, const tinygltf::Model& model);
    static void setupMaterialFlags(AssetRef<MaterialInstance> mat, const tinygltf::Material& material);

    static AssetRef<MaterialInstance> processMaterial(const tinygltf::Material& material, const tinygltf::Model& model);
    static Mesh* createMesh(const tinygltf::Mesh &mesh, const tinygltf::Model &model);

    static std::vector<glm::vec3> extractVertexPositions(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<glm::vec3> extractVertexNormals(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<glm::vec2> extractVertexUVs(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<uint32_t> extractIndices(const tinygltf::Primitive& primitive, const tinygltf::Model& model);

    static AssetRef<Image> loadTexture(const tinygltf::Texture& texture, const tinygltf::Model& model, bool isSrgb);
};
}
