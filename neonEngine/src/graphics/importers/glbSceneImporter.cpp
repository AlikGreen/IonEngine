#include "glbSceneImporter.h"

#include <tiny_gltf.h>

#include "graphics/components/meshRenderer.h"
#include "core/engine.h"
#include "glm/gtc/type_ptr.hpp"
#include "graphics/renderSystem.h"

#include <neonRHI/neonRHI.h>
#include <neonLog/neonLog.h>


#include "core/scene.h"
#include "core/components/transformComponent.h"
#include "graphics/image.h"

namespace Neon
{
    void* GLBSceneImporter::load(const std::string& filePath)
    {
        tinygltf::Model model;
        if (!loadModel(model, filePath))
        {
            return nullptr;
        }

        const auto scene = new Scene();
        ECS::Entity rootEntity = scene->createEntity();

        const auto materials = processMaterials(model);
        const AssetRef<MaterialShader> defaultMaterial = Engine::getAssetManager().addAsset(MaterialShader::createPBR(), "Default Material");
        processNodes(model, *scene, materials, defaultMaterial);

        return scene;
    }

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
            Log::warning(warn);
        }
        if (!err.empty())
        {
            Log::error("Failed to load model\nPath:{}\nError: {}", filePath, err);
        }

        return success;
    }

    std::vector<AssetRef<MaterialShader>> GLBSceneImporter::processMaterials(const tinygltf::Model& model)
    {
        std::vector<AssetRef<MaterialShader>> materials;
        materials.reserve(model.materials.size());

        for (const auto& material : model.materials)
        {
            materials.push_back(processMaterial(material, model));
        }

        return materials;
    }

    void GLBSceneImporter::processNodes(const tinygltf::Model& model, Scene& scene, const std::vector<AssetRef<MaterialShader>>& materials, const AssetRef<MaterialShader>& defaultMaterial)
    {
        for (const auto& node : model.nodes)
        {
            if (node.mesh < 0) continue;

            const tinygltf::Mesh& mesh = model.meshes[node.mesh];
            const auto nMesh = createMesh(mesh, model);
            if (!nMesh) continue;

            const auto meshHandle = Engine::getAssetManager().addAsset(nMesh, mesh.name);
            ECS::Entity entity = scene.createEntity();

            setupTransform(entity, node);
            setupMeshRenderer(entity, meshHandle, mesh, defaultMaterial, materials);
        }
    }

    void GLBSceneImporter::setupTransform(ECS::Entity& entity, const tinygltf::Node& node)
    {
        auto& transform = entity.get<Transform>();

        if (node.translation.size() == 3)
        {
            transform.setPosition(glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        }
        if (node.rotation.size() == 4)
        {
            transform.setRotation(eulerAngles(glm::quat(
                static_cast<float>(node.rotation[3]),
                static_cast<float>(node.rotation[0]),
                static_cast<float>(node.rotation[1]),
                static_cast<float>(node.rotation[2]))));
        }
        if (node.scale.size() == 3)
        {
            transform.setScale(glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        }
    }

    void GLBSceneImporter::setupMeshRenderer(ECS::Entity& entity, const AssetRef<Mesh> meshHandle, const tinygltf::Mesh& mesh, const AssetRef<MaterialShader>& defaultMaterial, const std::vector<AssetRef<MaterialShader>>& materials)
    {
        auto& meshRenderer = entity.emplace<MeshRenderer>();
        meshRenderer.mesh = meshHandle;
        meshRenderer.mesh->apply();

        if (mesh.primitives.empty() || mesh.primitives[0].material < 0)
        {
            meshRenderer.setMaterial(defaultMaterial);
        }
        else
        {
            for (const auto& primitive : mesh.primitives)
            {
                meshRenderer.materials.emplace_back(materials[primitive.material]);
            }
        }
    }

    AssetRef<MaterialShader> GLBSceneImporter::processMaterial(const tinygltf::Material& material, const tinygltf::Model& model)
    {
        auto mat = MaterialShader::createPBR();
        // mat.name = material.name;

        setupPBRProperties(mat, material, model);
        setupTextureProperties(mat, material, model);
        setupMaterialFlags(mat, material);

        return Engine::getAssetManager().addAsset(mat, material.name);
    }

    void GLBSceneImporter::setupPBRProperties(MaterialShader& mat, const tinygltf::Material& material, const tinygltf::Model& model)
    {
        const Rc<RHI::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();
        const auto& pbr = material.pbrMetallicRoughness;

        if (pbr.baseColorTexture.index >= 0)
        {
            const AssetRef<Image> imageRef = loadTexture(model.textures[pbr.baseColorTexture.index], model, true);
            Rc<RHI::TextureView> view = device->createTextureView(RHI::TextureViewDesc(imageRef->texture));
            mat.setTexture("albedoTexture", view);
            mat.setSampler("albedoSampler", imageRef->sampler);
        }

        const auto& baseColor = pbr.baseColorFactor;
        mat.setProperty("matAlbedo", glm::vec4(baseColor[0], baseColor[1], baseColor[2], baseColor[3]));
        // mat.setProperty("matMetalness", static_cast<float>(pbr.metallicFactor));
        mat.setProperty("matRoughness", static_cast<float>(pbr.roughnessFactor));

        if (pbr.metallicRoughnessTexture.index >= 0)
        {
            const AssetRef<Image> imageRef = loadTexture(model.textures[pbr.metallicRoughnessTexture.index], model, false);
            Rc<RHI::TextureView> view = device->createTextureView(RHI::TextureViewDesc(imageRef->texture));
            mat.setTexture("metallicRoughnessTexture", view);
            mat.setSampler("metallicRoughnessSampler", imageRef->sampler);
        }
    }

    void GLBSceneImporter::setupTextureProperties(MaterialShader& mat, const tinygltf::Material& material, const tinygltf::Model& model)
    {
        const Rc<RHI::Device> device = Engine::getSystem<GraphicsSystem>()->getDevice();

        if (material.normalTexture.index >= 0)
        {
            const AssetRef<Image> imageRef = loadTexture(model.textures[material.normalTexture.index], model, false);
            Rc<RHI::TextureView> view = device->createTextureView(RHI::TextureViewDesc(imageRef->texture));
            mat.setTexture("normalTexture", view);
            mat.setSampler("normalSampler", imageRef->sampler);

            // mat.setProperty("normalTextureStrength", static_cast<float>(material.normalTexture.scale));
        }

        if (material.occlusionTexture.index >= 0)
        {
            const AssetRef<Image> imageRef = loadTexture(model.textures[material.occlusionTexture.index], model, false);
            const Rc<RHI::TextureView> view = device->createTextureView(RHI::TextureViewDesc(imageRef->texture));
            mat.setTexture("occlusionTexture", view);
            mat.setSampler("occlusionSampler", imageRef->sampler);

            mat.setProperty("occlusionTextureStrength", static_cast<float>(material.occlusionTexture.strength));
        }

        if (material.emissiveTexture.index >= 0)
        {
            const AssetRef<Image> imageRef = loadTexture(model.textures[material.emissiveTexture.index], model, false);
            const Rc<RHI::TextureView> view = device->createTextureView(RHI::TextureViewDesc(imageRef->texture));
            mat.setTexture("emissionTexture", view);
            mat.setSampler("emissionSampler", imageRef->sampler);
        }

        const auto& emission = material.emissiveFactor;
        // mat.setProperty("emission", glm::vec3(emission[0], emission[1], emission[2]));
    }

    void GLBSceneImporter::setupMaterialFlags(MaterialShader& mat, const tinygltf::Material& material)
    {
        // mat.setProperty("alphaCutoff", static_cast<float>(material.alphaCutoff));
        // mat.setProperty<int>("doubleSided", material.doubleSided);
    }

    RHI::TextureFilter convertGLTFFilter(const int gltfFilter)
    {
        using namespace RHI;
        switch (gltfFilter)
        {
            case TINYGLTF_TEXTURE_FILTER_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
                return TextureFilter::Nearest;
            case TINYGLTF_TEXTURE_FILTER_LINEAR:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            default:
                return TextureFilter::Linear;
        }
    }

    RHI::MipmapFilter convertGLTFMipmapFilter(const int gltfFilter)
    {
        using namespace RHI;
        switch (gltfFilter)
        {
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
                return MipmapFilter::Nearest;
            case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
                return MipmapFilter::Linear;
            default:
                return MipmapFilter::None;
        }
    }

    RHI::TextureWrap convertGLTFWrap(const int gltfWrap)
    {
        using namespace RHI;
        switch (gltfWrap)
        {
            case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                return TextureWrap::MirroredRepeat;
            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
                return TextureWrap::ClampToEdge;
            case TINYGLTF_TEXTURE_WRAP_REPEAT:
            default:
                return TextureWrap::Repeat;
        }
    }

    RHI::PixelFormat determineTextureFormat(const tinygltf::Image& image, const bool isSRGB)
    {
        using namespace RHI;
        const auto components = image.component;
        const auto bits = image.bits;

        if (components == 1)
        {
            if (bits == 8) return PixelFormat::R8Unorm;
            if (bits == 16) return PixelFormat::R16Unorm;
            if (bits == 32) return PixelFormat::R32Float;
        }
        else if (components == 2)
        {
            if (bits == 8) return PixelFormat::R8G8Unorm;
            if (bits == 16) return PixelFormat::R16G16Unorm;
            if (bits == 32) return PixelFormat::R32G32Float;
        }
        else if (components == 3 || components == 4)
        {
            if (bits == 8) return isSRGB ? PixelFormat::R8G8B8A8UnormSrgb : PixelFormat::R8G8B8A8Unorm;
            if (bits == 16) return PixelFormat::R16G16B16A16Unorm;
            if (bits == 32) return PixelFormat::R32G32B32A32Float;
        }

        return PixelFormat::R8G8B8A8Unorm;
    }

    AssetRef<Image> GLBSceneImporter::loadTexture(const tinygltf::Texture& texture, const tinygltf::Model& model, const bool isSrgb)
    {
        if (texture.source < 0)
        {
            return nullptr;
        }

        const tinygltf::Image& image = model.images[texture.source];
        const std::vector<unsigned char>& data = image.image;

        RHI::TextureDesc textureDescription{};
        textureDescription.format = determineTextureFormat(image, isSrgb);
        textureDescription.width = image.width;
        textureDescription.height = image.height;

        RHI::SamplerDesc samplerDescription{};

        if (texture.sampler >= 0)
        {
            const tinygltf::Sampler& sampler = model.samplers[texture.sampler];
            samplerDescription.minFilter = convertGLTFFilter(sampler.minFilter);
            samplerDescription.magFilter = convertGLTFFilter(sampler.magFilter);
            samplerDescription.wrapMode.x = convertGLTFWrap(sampler.wrapS);
            samplerDescription.wrapMode.y = convertGLTFWrap(sampler.wrapT);
        }

        const Rc<RHI::Device>& device = Engine::getSystem<GraphicsSystem>()->getDevice();
        const Rc<RHI::Texture>& tex = device->createTexture(textureDescription);
        const Rc<RHI::Sampler>& sampler = device->createSampler(samplerDescription);

        AssetManager& assetManager = Engine::getAssetManager();

        RHI::TextureUploadDesc uploadDescription{};
        uploadDescription.data = data.data();
        uploadDescription.pixelType = tinyGltfGetPixelType(image);
        uploadDescription.width = image.width;
        uploadDescription.height = image.height;

        const auto commandList = device->createCommandList();

        commandList->begin();
        commandList->updateTexture(tex, uploadDescription);
        commandList->generateMipmaps(tex);
        device->submit(commandList);

        return assetManager.addAsset(Image(tex, sampler), texture.name);
    }

    Mesh* GLBSceneImporter::createMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model)
    {
        if (mesh.primitives.empty())
        {
            return nullptr;
        }

        std::vector<Vertex> vertices{};
        std::vector<uint32_t> meshIndices{};

        auto nMesh = std::make_unique<Mesh>();

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

        nMesh->setIndices(meshIndices);
        nMesh->setVertices(vertices);
        return nMesh.release();
    }


    std::vector<glm::vec3> GLBSceneImporter::extractVertexPositions(const tinygltf::Primitive& primitive, const tinygltf::Model& model)
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

    std::vector<glm::vec3> GLBSceneImporter::extractVertexNormals(const tinygltf::Primitive& primitive, const tinygltf::Model& model)
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

    std::vector<glm::vec2> GLBSceneImporter::extractVertexUVs(const tinygltf::Primitive& primitive, const tinygltf::Model& model)
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

    std::vector<uint32_t> GLBSceneImporter::extractIndices(const tinygltf::Primitive& primitive, const tinygltf::Model& model)
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


    RHI::PixelType GLBSceneImporter::tinyGltfGetPixelType(const tinygltf::Image &img)
    {
        switch (img.pixel_type)
        {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                return RHI::PixelType::UnsignedByte;
            case TINYGLTF_COMPONENT_TYPE_BYTE:
                return RHI::PixelType::Byte;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                return RHI::PixelType::UnsignedShort;
            case TINYGLTF_COMPONENT_TYPE_SHORT:
                return RHI::PixelType::Short;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                return RHI::PixelType::UnsignedInt;
            case TINYGLTF_COMPONENT_TYPE_INT:
                return RHI::PixelType::Int;
            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                return RHI::PixelType::Float;
            default:
                    if (img.bits == 16) return RHI::PixelType::UnsignedShort;
            if (img.bits == 32) return RHI::PixelType::Float;
            return RHI::PixelType::UnsignedByte;
        }
    }
}
