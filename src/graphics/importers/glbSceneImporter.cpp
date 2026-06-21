#include "glbSceneImporter.h"

#include <tiny_gltf.h>

#include "asset/assetRegistry.h"
#include "core/engine.h"
#include "core/components/parentComponent.h"
#include "core/components/tagComponent.h"
#include "core/components/transformComponent.h"
#include "glm/gtc/quaternion.hpp"
#include "graphics/graphicsSystem.h"
#include "graphics/materialInstance.h"
#include "graphics/components/meshRenderer.h"

namespace ion
{
    bool isGlbFile(std::filesystem::path const& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return false;

        char magic[4]{};
        if (!file.read(magic, 4))
            return false;

        return magic[0] == 'g'
            && magic[1] == 'l'
            && magic[2] == 'T'
            && magic[3] == 'F';
    }

    grl::Box<Scene> GLBSceneImporter::import(const std::filesystem::path &filePath, const NoOptions &)
    {
        tinygltf::Model model;
        if (!loadModel(model, filePath.string()))
        {
            return nullptr;
        }

        auto scene = grl::makeBox<Scene>();
        scene->name = filePath.stem().string();

        static AssetRef<MaterialInstance> defaultMaterial = Engine::assetRegistry().create<MaterialInstance>(MaterialTemplates::pbr());

        auto materials = extractMaterials(model);
        materials.push_back(defaultMaterial);

        const auto meshes = extractMeshes(model);

        processNodes(*scene, materials, meshes, model);

        return scene;
    }

    bool GLBSceneImporter::canImport(const std::filesystem::path &src) const
    {
        return src.extension() == ".glb" || src.extension() == ".gltf" || isGlbFile(src);
    }

    bool GLBSceneImporter::loadModel(tinygltf::Model& model, const std::string& filePath)
    {
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        bool success = false;

        if(isGlbFile(filePath))
            success = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);
        else
            success = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);

        if (!warn.empty())
        {
            clogr::warn(warn);
        }
        if (!err.empty())
        {
            clogr::error("Failed to load model\nPath:{}\nError: {}", filePath, err);
        }

        return success;
    }

    std::vector<AssetRef<MaterialInstance>> GLBSceneImporter::extractMaterials(const tinygltf::Model &model)
    {
        std::vector<AssetRef<MaterialInstance>> materials{};
        materials.reserve(model.materials.size());

        for(const auto& gltfMat : model.materials)
        {
            auto material = processMaterial(model, gltfMat);
            materials.push_back(material);
        }

        return materials;
    }

    AssetRef<MaterialInstance> GLBSceneImporter::processMaterial(const tinygltf::Model &model, tinygltf::Material gltfMat)
    {
        const grl::Rc<urhi::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();
        auto& assets = Engine::assetRegistry();

        auto material = assets.create<MaterialInstance>(MaterialTemplates::pbr());
        assets.setName(material.id(), gltfMat.name);

        const auto& pbr = gltfMat.pbrMetallicRoughness;

        if (pbr.baseColorTexture.index >= 0)
        {
            const AssetRef<Image> imageRef = loadTexture(model.textures[pbr.baseColorTexture.index], model, true);
            material->setTexture("albedoMap", imageRef);
            material->setSampler("albedoSampler", imageRef);
        }

        const auto& baseColor = pbr.baseColorFactor;
        material->set("baseColor", glm::vec4(baseColor[0], baseColor[1], baseColor[2], baseColor[3]));
        // mat.setProperty("matMetalness", static_cast<float>(pbr.metallicFactor));
        material->set("roughness", static_cast<float>(pbr.roughnessFactor));

        if (pbr.metallicRoughnessTexture.index >= 0)
        {
            const AssetRef<Image> imageRef = loadTexture(model.textures[pbr.metallicRoughnessTexture.index], model, false);
            material->setTexture("metallicRoughnessMap", imageRef);
            material->setSampler("metallicRoughnessSampler", imageRef);
        }

        // TODO set other properties and textures eg normal, occlusion and emission

        return material;
    }

    std::vector<AssetRef<Mesh>> GLBSceneImporter::extractMeshes(tinygltf::Model &model)
    {
        std::vector<AssetRef<Mesh>> meshes{};
        meshes.reserve(model.meshes.size());

        for(const auto& gltfMesh : model.meshes)
        {
            auto mesh = processMesh(gltfMesh, model);
            meshes.push_back(mesh);
        }

        return meshes;
    }

    AssetRef<Mesh> GLBSceneImporter::processMesh(const tinygltf::Mesh &mesh, const tinygltf::Model &model)
    {
        if (mesh.primitives.empty())
        {
            return nullptr;
        }

        std::vector<Vertex> vertices{};
        std::vector<uint32_t> meshIndices{};

        auto& assets = Engine::assetRegistry();
        auto nMesh = assets.create<Mesh>();
        assets.setName(nMesh.id(), mesh.name);


        for(const auto& primitive : mesh.primitives)
        {
            const auto positions = extractVertexPositions(primitive, model);

            const uint32_t startingVerticesCount = vertices.size();

            vertices.resize(startingVerticesCount+positions.size());

            for(int i = 0; i < positions.size(); i++)
            {
                vertices[startingVerticesCount+i].position = positions[i];
            }

            const auto normals = extractVertexNormals(primitive, model);
            if(normals.size() == positions.size())
            {
                for(int i = 0; i < normals.size(); i++)
                {
                    vertices[startingVerticesCount+i].normal = normals[i];
                }
            }

            const auto uvs = extractVertexUVs(primitive, model);
            if(uvs.size() == positions.size())
            {
                for(int i = 0; i < uvs.size(); i++)
                {
                    vertices[startingVerticesCount+i].uv = uvs[i];
                }
            }

            auto indices = extractIndices(primitive, model);
            const uint32_t startingIndicesCount = meshIndices.size();
            meshIndices.resize(startingIndicesCount+indices.size());

            nMesh->addPrimitive(startingIndicesCount, indices.size());

            for(int i = 0; i < indices.size(); i++)
            {
                meshIndices[startingIndicesCount+i] = startingVerticesCount+indices[i];
            }
        }

        nMesh->indices(meshIndices);
        nMesh->vertices(vertices);
        return nMesh;
    }

    std::vector<glm::vec3> GLBSceneImporter::extractVertexPositions(const tinygltf::Primitive &primitive, const tinygltf::Model &model)
    {
        const auto it = primitive.attributes.find("POSITION");
        if (it == primitive.attributes.end())
        {
            return {};
        }

        const auto& accessor = model.accessors[it->second];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];

        const auto* data = reinterpret_cast<const float*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

        std::vector<glm::vec3> positions{};
        positions.resize(accessor.count);

        for (size_t i = 0; i < accessor.count; ++i)
        {
            positions[i] = glm::vec3(data[3 * i], data[3 * i + 1], data[3 * i + 2]);
        }

        return positions;
    }

    std::vector<glm::vec3> GLBSceneImporter::extractVertexNormals(const tinygltf::Primitive &primitive, const tinygltf::Model &model)
    {
        const auto it = primitive.attributes.find("NORMAL");
        if (it == primitive.attributes.end())
        {
            return {};
        }

        const auto& accessor = model.accessors[it->second];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];

        const auto* data = reinterpret_cast<const float*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

        std::vector<glm::vec3> normals{};
        normals.resize(accessor.count);

        for (size_t i = 0; i < accessor.count; ++i)
        {
            normals[i] = glm::vec3(data[3 * i], data[3 * i + 1], data[3 * i + 2]);
        }

        return normals;
    }

    std::vector<glm::vec2> GLBSceneImporter::extractVertexUVs(const tinygltf::Primitive &primitive, const tinygltf::Model &model)
    {
        const auto it = primitive.attributes.find("TEXCOORD_0");
        if (it == primitive.attributes.end())
        {
            return{};
        }

        const auto& accessor = model.accessors[it->second];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];

        const auto* data = reinterpret_cast<const float*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);

        std::vector<glm::vec2> uvs{};
        uvs.resize(accessor.count);

        for (size_t i = 0; i < accessor.count; ++i)
        {
            uvs[i] = glm::vec2(data[2 * i], data[2 * i + 1]);
        }

        return uvs;
    }

    std::vector<uint32_t> GLBSceneImporter::extractIndices(const tinygltf::Primitive &primitive, const tinygltf::Model &model)
    {
        if (primitive.indices < 0)
        {
            return{};
        }

        const auto& accessor = model.accessors[primitive.indices];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];

        std::vector<uint32_t> newIndices{};
        newIndices.resize(accessor.count);

        const auto* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            const auto* indices = reinterpret_cast<const uint16_t*>(data);
            for (size_t i = 0; i < accessor.count; ++i)
            {
                newIndices[i] = static_cast<uint32_t>(indices[i]);
            }
        }
        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
        {
            const auto* indices = reinterpret_cast<const uint32_t*>(data);
            for (size_t i = 0; i < accessor.count; ++i)
            {
                newIndices[i] = indices[i];
            }
        }

        return newIndices;
    }

    urhi::PixelFormat determineTextureFormat(const tinygltf::Image& image, const bool isSRGB)
    {
        using namespace urhi;
        const auto components = image.component;
        const auto bits = image.bits;

        if (components == 1)
        {
            if (bits == 8) return PixelFormat::R8UNorm;
            if (bits == 16) return PixelFormat::R16Float;
            if (bits == 32) return PixelFormat::R32Float;
        }
        else if (components == 2)
        {
            if (bits == 8) return PixelFormat::RG8UNorm;
            if (bits == 16) return PixelFormat::RG16Float;
            if (bits == 32) return PixelFormat::RG32Float;
        }
        else if (components == 3 || components == 4)
        {
            if (bits == 8) return isSRGB ? PixelFormat::RGBA8UNormSrgb : PixelFormat::RGBA8UNorm;
            if (bits == 16) return PixelFormat::RGBA16Float;
            if (bits == 32) return PixelFormat::RGBA32Float;
        }

        return PixelFormat::RGBA8UNorm;
    }

    AssetRef<Image> GLBSceneImporter::loadTexture(const tinygltf::Texture &texture, const tinygltf::Model &model, bool isSrgb)
    {
        if (texture.source < 0)
            return nullptr;

        const tinygltf::Image& image = model.images[texture.source];
        const std::vector<uint8_t>& data = image.image;

        // if (texture.sampler >= 0)
        // {
        //     const tinygltf::Sampler& sampler = model.samplers[texture.sampler];
        //     samplerDesc.minFilter = convertGLTFFilter(sampler.minFilter);
        //     samplerDesc.magFilter = convertGLTFFilter(sampler.magFilter);
        //     samplerDesc.addressModeU = convertGLTFWrap(sampler.wrapS);
        //     samplerDesc.addressModeV = convertGLTFWrap(sampler.wrapT);
        // }

        TextureData texData{};
        texData.data.resize(data.size());
        std::memcpy(texData.data.data(), data.data(), data.size());
        texData.width = image.width;
        texData.height = image.height;
        texData.pixelFormat = determineTextureFormat(image, isSrgb);

        auto& registry = Engine::assetRegistry();
        auto asset = registry.create<Image>(texData);
        registry.setName(asset.id(), texture.name);
        return asset;
    }

    void GLBSceneImporter::processNodes(
        Scene &scene,
        const std::vector<AssetRef<MaterialInstance>> &materials,
        const std::vector<AssetRef<Mesh>> &meshes,
        const tinygltf::Model &model)
    {
        std::unordered_map<int, entis::Entity> nodeEntityMap;

        for (int i = 0; i < static_cast<int>(model.nodes.size()); ++i)
        {
            auto node = model.nodes[i];

            entis::Entity entity = scene.createEntity();
            entity.get<Tag>().name = node.name;
            nodeEntityMap.emplace(i, entity);

            setupTransform(entity, node);

            if (node.mesh < 0 || node.mesh >= meshes.size())
                continue;

            setupMeshRenderer(entity, meshes.at(node.mesh), model.meshes[node.mesh], materials);
        }

        for (int i = 0; i < static_cast<int>(model.nodes.size()); ++i)
        {
            const tinygltf::Node& node = model.nodes[i];
            const entis::Entity parent = nodeEntityMap.at(i);;

            for (int childIdx : node.children)
            {
                entis::Entity child = nodeEntityMap.at(childIdx);
                child.get<Parent>().setParent(parent);
            }
        }
    }

    void GLBSceneImporter::setupTransform(entis::Entity& entity, const tinygltf::Node& node)
    {
        auto& transform = entity.get<Transform>();

        if (node.translation.size() == 3)
        {
            transform.position(glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        }
        if (node.rotation.size() == 4)
        {
            transform.rotation(glm::quat(
                static_cast<float>(node.rotation[3]),
                static_cast<float>(node.rotation[0]),
                static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2])));
        }
        if (node.scale.size() == 3)
        {
            transform.scale(glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        }
    }

    void GLBSceneImporter::setupMeshRenderer(
        entis::Entity& entity,
        const AssetRef<Mesh>& meshHandle,
        const tinygltf::Mesh& mesh,
        const std::vector<AssetRef<MaterialInstance>>& materials)
    {
        auto& meshRenderer = entity.emplace<MeshRenderer>();
        meshRenderer.mesh = meshHandle;


        if (mesh.primitives.empty())
        {
            meshRenderer.setMaterial(materials.back());
        }

        for (const auto& primitive : mesh.primitives)
        {
            if(primitive.material < 0 || primitive.material >= materials.size()-1)
            {
                meshRenderer.materials.emplace_back(materials.back());
            }else
            {
                meshRenderer.materials.emplace_back(materials[primitive.material]);
            }
        }

    }
}
