#pragma once
#include <tiny_gltf.h>

#include "asset/assetImporter.h"
#include "core/scene.h"
#include "graphics/image.h"
#include "graphics/assets/materialShader.h"
#include "graphics/assets/mesh.h"

namespace ion
{
class GLBSceneImporter final : public AssetImporter
{
public:
    void* load(const std::string& filePath) override;

private:

    static bool loadModel(tinygltf::Model& model, const std::string& filePath);
    static std::vector<AssetRef<MaterialShader>> processMaterials(const tinygltf::Model& model);
    static void processNodes(const tinygltf::Model& model, Scene& scene, const std::vector<AssetRef<MaterialShader>>& materials, const AssetRef<MaterialShader>& defaultMaterial);

    static void setupTransform(entis::Entity& entity, const tinygltf::Node& node);
    static void setupMeshRenderer(entis::Entity& entity, AssetRef<Mesh> meshHandle, const tinygltf::Mesh& mesh, const AssetRef<MaterialShader>& defaultMaterial, const std::vector<AssetRef<MaterialShader>>& materials);
    static void setupPBRProperties(MaterialShader& mat, const tinygltf::Material& material, const tinygltf::Model& model);
    static void setupTextureProperties(MaterialShader& mat, const tinygltf::Material& material, const tinygltf::Model& model);
    static void setupMaterialFlags(MaterialShader& mat, const tinygltf::Material& material);

    static AssetRef<MaterialShader> processMaterial(const tinygltf::Material& material, const tinygltf::Model& model);
    static Mesh* createMesh(const tinygltf::Mesh &mesh, const tinygltf::Model &model);

    static std::vector<glm::vec3> extractVertexPositions(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<glm::vec3> extractVertexNormals(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<glm::vec2> extractVertexUVs(const tinygltf::Primitive& primitive, const tinygltf::Model& model);
    static std::vector<uint32_t> extractIndices(const tinygltf::Primitive& primitive, const tinygltf::Model& model);

    static AssetRef<Image> loadTexture(const tinygltf::Texture& texture, const tinygltf::Model& model, bool isSrgb);

    // static urhi::PixelFormat tinyGltfGetPixelFormat(const tinygltf::Image &img);
    static urhi::PixelType tinyGltfGetPixelType(const tinygltf::Image &img);
};
}
