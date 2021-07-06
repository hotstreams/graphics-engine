#include <limitless/loaders/model_loader.hpp>

#include <limitless/ms/material_builder.hpp>
#include <limitless/loaders/texture_loader.hpp>
#include <limitless/models/skeletal_model.hpp>
#include <limitless/assets.hpp>

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <glm/gtx/quaternion.hpp>
#include <limitless/util/glm.hpp>

using namespace Limitless;

std::shared_ptr<AbstractModel> ModelLoader::loadModel(Assets& assets, const fs::path& _path, const ModelLoaderFlags& flags) {
    const auto path = convertPathSeparators(_path);

    Assimp::Importer importer;
    const aiScene* scene;

    auto scene_flags = aiProcess_ValidateDataStructure |
                       aiProcess_Triangulate |
                       aiProcess_GenUVCoords |
                       aiProcess_GenNormals |
                       aiProcess_GenSmoothNormals |
                       aiProcess_CalcTangentSpace |
                       aiProcess_ImproveCacheLocality;

    if (flags.find(ModelLoaderFlag::FlipUV) != flags.end()) {
        scene_flags |= aiProcess_FlipUVs;
    }

    if (flags.find(ModelLoaderFlag::FlipWindingOrder) != flags.end()) {
        scene_flags |= aiProcess_FlipWindingOrder;
    }

    scene = importer.ReadFile(path.string().c_str(), scene_flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw model_loader_error(importer.GetErrorString());
    }

    std::unordered_map<std::string, uint32_t> bone_map;
    std::vector<Bone> bones;

    auto meshes = loadMeshes(assets, scene, path, bones, bone_map, flags);

    std::vector<std::shared_ptr<ms::Material>> materials;
    if (!flags.count(ModelLoaderFlag::NoMaterials)) {
        materials = loadMaterials(assets, scene, path, bone_map.empty() ? ModelShader::Model : ModelShader::Skeletal);
    }

    auto animations = loadAnimations(scene, bones, bone_map, flags);

    auto animation_tree = loadAnimationTree(scene, bones, bone_map, flags);

    auto global_matrix = convert(scene->mRootNode->mTransformation);

    if (flags.find(ModelLoaderFlag::FlipYZ) != flags.end()) {
        global_matrix = flipYZ(global_matrix);
    }

    importer.FreeScene();

    auto model = bone_map.empty() ?
        std::shared_ptr<AbstractModel>(new Model(std::move(meshes), std::move(materials), path.stem().string())) :
        std::shared_ptr<AbstractModel>(new SkeletalModel(std::move(meshes), std::move(materials), std::move(bones), std::move(bone_map), std::move(animation_tree), std::move(animations), glm::inverse(global_matrix), path.stem().string()));

    return model;
}

template<typename T>
std::vector<T> ModelLoader::loadVertices(aiMesh* mesh, const ModelLoaderFlags& flags) noexcept {
    std::vector<T> vertices;
    vertices.reserve(mesh->mNumVertices);

    for (uint32_t j = 0; j < mesh->mNumVertices; ++j) {
        auto vertex = convert3f(mesh->mVertices[j]);
        auto normal = convert3f(mesh->mNormals[j]);
        auto tangent = convert3f(mesh->mTangents[j]);
        auto uv = convert2f(mesh->mTextureCoords[0][j]);

        if (flags.find(ModelLoaderFlag::FlipYZ) != flags.end()) {
            vertex = flipYZ(vertex);
            normal = flipYZ(normal);
            tangent = flipYZ(tangent);
        }

        if constexpr (std::is_same<T, VertexNormalTangent>::value) {
            vertices.emplace_back(T{vertex, normal, tangent, uv});
        }

        if constexpr (std::is_same<T, VertexPackedNormalTangent>::value) {
            auto packed_normal = pack(normal);
            auto packed_tangent = pack(tangent);
            auto packed_uv = glm::packHalf2x16(uv);

            vertices.emplace_back(T{vertex, packed_normal, packed_tangent, packed_uv});
        }
    }

    return vertices;
}

template<typename T>
std::vector<T> ModelLoader::loadIndices(aiMesh* mesh) noexcept {
    std::vector<T> indices;
    indices.reserve(mesh->mNumFaces * 3);

    for (uint32_t k = 0; k < mesh->mNumFaces; ++k) {
        const auto face = mesh->mFaces[k];
        indices.emplace_back(face.mIndices[0]);
        indices.emplace_back(face.mIndices[1]);
        indices.emplace_back(face.mIndices[2]);
    }

    return indices;
}

template<typename T, typename T1>
std::shared_ptr<AbstractMesh> ModelLoader::loadMesh(
        Assets& assets,
        aiMesh *m,
        const fs::path& path,
        std::vector<Bone>& bones,
        std::unordered_map<std::string, uint32_t>& bone_map,
        const ModelLoaderFlags& flags) {
    static auto i = 0;
	static size_t unnamed_mesh_index = 0;
    auto mesh_name = m->mName.length != 0 ? m->mName.C_Str() : std::to_string(i++);
    std::string name = path.string() + PATH_SEPARATOR + mesh_name;

    if (flags.find(ModelLoaderFlag::GenerateUniqueMeshNames) != flags.end()) {
        name += std::to_string(unnamed_mesh_index++);
    }

    if (assets.meshes.contains(name)) {
        return assets.meshes.at(name);
    }

    auto vertices = loadVertices<T>(m, flags);
    auto indices = loadIndices<T1>(m);
    auto weights = loadBoneWeights(m, bones, bone_map, flags);

    auto mesh = bone_map.empty() ?
        std::shared_ptr<AbstractMesh>(new IndexedMesh<T, T1>(std::move(vertices), std::move(indices), std::move(name), MeshDataType::Static, DrawMode::Triangles)) :
        std::shared_ptr<AbstractMesh>(new SkinnedMesh<T, T1>(std::move(vertices), std::move(indices), std::move(weights), std::move(name), MeshDataType::Static, DrawMode::Triangles));

    assets.meshes.add(mesh->getName(), mesh);

    return mesh;
}

std::shared_ptr<ms::Material> ModelLoader::loadMaterial(
        Assets& assets,
        aiMaterial* mat,
        const fs::path& path,
        const ModelShaders& model_shaders) {
    aiString aname;
    mat->Get(AI_MATKEY_NAME, aname);

    auto path_str = path.parent_path().string();
    static auto i = 0;
    auto mat_name = aname.length != 0 ? aname.C_Str() : std::to_string(i++);
    auto name = path_str + PATH_SEPARATOR + mat_name;

    if (assets.materials.contains(name)) {
        return assets.materials.at(name);
    }

    ms::MaterialBuilder builder {assets};

    builder.setName(std::move(name))
           .setShading(ms::Shading::Lit);

    if (auto diffuse_count = mat->GetTextureCount(aiTextureType_DIFFUSE); diffuse_count != 0) {
        aiString texture_name;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &texture_name);

        builder.add(ms::Property::Diffuse, TextureLoader::load(assets, path_str + PATH_SEPARATOR + texture_name.C_Str(), {TextureLoaderFlags::Space::sRGB}));
    }

    if (auto normal_count = mat->GetTextureCount(aiTextureType_HEIGHT); normal_count != 0) {
        aiString texture_name;
        mat->GetTexture(aiTextureType_HEIGHT, 0, &texture_name);

        builder.add(ms::Property::Normal, TextureLoader::load(assets, path_str + PATH_SEPARATOR + texture_name.C_Str()));
    }

    if (auto specular_count = mat->GetTextureCount(aiTextureType_SPECULAR); specular_count != 0) {
        aiString texture_name;
        mat->GetTexture(aiTextureType_SPECULAR, 0, &texture_name);

        builder.add(ms::Property::Specular, TextureLoader::load(assets, path_str + PATH_SEPARATOR + texture_name.C_Str()));
    }

    float shininess = 16.0f;
    mat->Get(AI_MATKEY_SHININESS, shininess);
    builder.add(ms::Property::Shininess, shininess);

    if (auto opacity_mask = mat->GetTextureCount(aiTextureType_OPACITY); opacity_mask != 0) {
        aiString texture_name;
        mat->GetTexture(aiTextureType_OPACITY, 0, &texture_name);

        builder.add(ms::Property::BlendMask, TextureLoader::load(assets, path_str + PATH_SEPARATOR + texture_name.C_Str()));
    }

    if (auto emissive_mask = mat->GetTextureCount(aiTextureType_EMISSIVE); emissive_mask != 0) {
        aiString texture_name;
        mat->GetTexture(aiTextureType_EMISSIVE, 0, &texture_name);

        builder.add(ms::Property::EmissiveMask, TextureLoader::load(assets, path_str + PATH_SEPARATOR + texture_name.C_Str(), {TextureLoaderFlags::Space::sRGB}));
    }

    {
        aiBlendMode blending {aiBlendMode_Default};
        mat->Get(AI_MATKEY_BLEND_FUNC, blending);
        switch (blending) {
            case _aiBlendMode_Force32Bit:
            case aiBlendMode_Default:
                builder.setBlending(ms::Blending::Opaque);
                break;
            case aiBlendMode_Additive:
                builder.setBlending(ms::Blending::Additive);
                break;
        }
    }

    {
        int twosided {};

        mat->Get(AI_MATKEY_TWOSIDED, twosided);

        builder.setTwoSided(twosided);
    }

    {
        aiColor3D color{0.0f};
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);

        float opacity{1.0f};
        mat->Get(AI_MATKEY_OPACITY, opacity);

        if (opacity != 1.0f) {
            builder.setBlending(ms::Blending::Translucent);
        }

        if (color != aiColor3D{0.0f}) {
            builder.add(ms::Property::Color, glm::vec4{color.r, color.g, color.b, opacity});
        }
    }

    {
        aiColor3D color{0.0f};
        mat->Get(AI_MATKEY_COLOR_EMISSIVE, color);

        if (color != aiColor3D{0.0f}) {
            builder.add(ms::Property::EmissiveColor, glm::vec4{color.r, color.g, color.b, 1.0f});
            builder.setShading(ms::Shading::Unlit);
        }
    }

    builder.setModelShaders(model_shaders);
    return builder.build();
}

std::vector<VertexBoneWeight> ModelLoader::loadBoneWeights(aiMesh* mesh, std::vector<Bone>& bones, std::unordered_map<std::string, uint32_t>& bone_map, const ModelLoaderFlags& flags) {
    std::vector<VertexBoneWeight> bone_weights;

    if (mesh->HasBones()) {
        bone_weights.resize(mesh->mNumVertices);

        for (uint32_t j = 0; j < mesh->mNumBones; ++j) {
            std::string bone_name = mesh->mBones[j]->mName.C_Str();

            auto bi = bone_map.find(bone_name);
            auto offset_mat = convert(mesh->mBones[j]->mOffsetMatrix);

            if (flags.find(ModelLoaderFlag::FlipYZ) != flags.end()) {
                offset_mat = flipYZ(offset_mat);
//                offset_mat = flipYZTransformationMatrix(offset_mat);
            }

            uint32_t bone_index;
            if (bi == bone_map.end()) {
                bones.emplace_back(bone_name, offset_mat);
                bone_index = bones.size() - 1;
                bone_map.insert({bone_name, bone_index});
            } else {
                bone_index = bi->second;
            }

            for (uint32_t k = 0; k < mesh->mBones[j]->mNumWeights; ++k) {
                auto w = mesh->mBones[j]->mWeights[k];
                bone_weights.at(mesh->mBones[j]->mWeights[k].mVertexId).addBoneWeight(bone_index, w.mWeight);
            }
        }
    }

    if (!bone_map.empty() && (mesh->mNumVertices != bone_weights.size())) {
        throw model_loader_error("Bone weights for some vertices does not exist, model is corrupted.");
    }

    return bone_weights;
}

std::vector<std::shared_ptr<ms::Material>> ModelLoader::loadMaterials(Assets& assets, const aiScene* scene, const fs::path& path, ModelShader model_shader) {
    std::vector<std::shared_ptr<ms::Material>> materials;

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        const auto* mesh = scene->mMeshes[i];
        auto* material = scene->mMaterials[mesh->mMaterialIndex];
        materials.emplace_back(loadMaterial(assets, material, path, {model_shader}));
    }

    return materials;
}

std::vector<Animation> ModelLoader::loadAnimations(const aiScene* scene, std::vector<Bone>& bones, std::unordered_map<std::string, uint32_t>& bone_map, const ModelLoaderFlags& flags) {
    std::vector<Animation> animations;
    for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
        const auto* anim = scene->mAnimations[i];

        std::string anim_name(anim->mName.C_Str());
        std::vector<AnimationNode> anim_nodes;

        for (uint32_t j = 0; j < anim->mNumChannels; ++j) {
            const auto* channel = anim->mChannels[j];

            auto bi = bone_map.find(channel->mNodeName.C_Str());
            if (bi == bone_map.end()) {
                bones.emplace_back(channel->mNodeName.C_Str(), glm::mat4(1.f));
                bone_map.emplace(channel->mNodeName.C_Str(), bones.size() - 1);
            }

            auto& bone = bones.at(bone_map.at(channel->mNodeName.C_Str()));
            std::vector<KeyFrame<glm::vec3>> pos_frames;
            std::vector<KeyFrame<glm::fquat>> rot_frames;
            std::vector<KeyFrame<glm::vec3>> scale_frames;

            for (uint32_t k = 0; k < channel->mNumPositionKeys; ++k) {
                auto vec = convert3f(channel->mPositionKeys[k].mValue);

                if (flags.find(ModelLoaderFlag::FlipYZ) != flags.end()) {
                    vec = flipYZ(vec);
                }

                pos_frames.emplace_back(vec, channel->mPositionKeys[k].mTime);
            }

            for (uint32_t k = 0; k < channel->mNumRotationKeys; ++k) {
                auto quat = convertQuat(channel->mRotationKeys[k].mValue);

                if (flags.find(ModelLoaderFlag::FlipYZ) != flags.end()) {
                    quat = flipYZ(quat);
                }

                rot_frames.emplace_back(quat, channel->mRotationKeys[k].mTime);
            }

            for (uint32_t k = 0; k < channel->mNumScalingKeys; ++k) {
                auto vec = convert3f(channel->mScalingKeys[k].mValue);

                if (flags.find(ModelLoaderFlag::FlipYZ) != flags.end()) {
                    vec = flipYZ(vec);
                }

                scale_frames.emplace_back(vec, channel->mScalingKeys[k].mTime);
            }

            anim_nodes.emplace_back(pos_frames, rot_frames, scale_frames, bone);
        }

        animations.emplace_back(anim_name, anim->mDuration, anim->mTicksPerSecond > 0 ? anim->mTicksPerSecond : 25, anim_nodes);
    }

    return animations;
}

Tree<uint32_t> ModelLoader::loadAnimationTree(const aiScene* scene, std::vector<Bone>& bones, std::unordered_map<std::string, uint32_t>& bone_map, const ModelLoaderFlags& flags) {
    auto bone_finder = [&] (const std::string& str){
        if (auto bi = bone_map.find(str); bi != bone_map.end()) {
            return bi->second;
        } else {
            bones.emplace_back(str, glm::mat4(1.f));
            bone_map.emplace(str, bones.size() - 1);
            return static_cast<uint32_t>(bones.size() - 1);
        }
    };

    Tree<uint32_t> tree(bone_finder(scene->mRootNode->mName.C_Str()));

    std::function<void(Tree<uint32_t>& tree, const aiNode*, int)> dfs;
    dfs = [&] (Tree<uint32_t>& tree, const aiNode* node, int depth) {
        bones[*tree].node_transform = convert(node->mTransformation);

        if (flags.find(ModelLoaderFlag::FlipYZ) != flags.end()) {
            bones[*tree].node_transform = flipYZTransformationMatrix(bones[*tree].node_transform);
            bones[*tree].node_transform = flipYZ(bones[*tree].node_transform);
        }

        for (uint32_t i = 0; i < node->mNumChildren; ++i) {
            auto& child = tree.add(bone_finder(node->mChildren[i]->mName.C_Str()));
            dfs(child, node->mChildren[i], depth + 1);
        }
    };

    dfs(tree, scene->mRootNode, 0);

    return tree;
}

std::vector<std::shared_ptr<AbstractMesh>> ModelLoader::loadMeshes(
        Assets& assets,
        const aiScene *scene,
        const fs::path& path,
        std::vector<Bone>& bones,
        std::unordered_map<std::string, uint32_t>& bone_map,
        const ModelLoaderFlags& flags) {
    std::vector<std::shared_ptr<AbstractMesh>> meshes;

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        auto* mesh = scene->mMeshes[i];

        std::shared_ptr<AbstractMesh> loaded_mesh;
        // specifies template arguments based on INDICES_COUNT & TANGENTS & format
//        if (auto indices_count = mesh->mNumFaces * 3; indices_count < std::numeric_limits<GLubyte>::max()) {
//            loaded_mesh = loadMesh<VertexNormalTangent, GLubyte>(mesh, path, bones, bone_map, flags);
//        } else if (indices_count < std::numeric_limits<GLushort>::max()) {
//            loaded_mesh = loadMesh<VertexNormalTangent, GLushort>(mesh, path, bones, bone_map, flags);
//        } else {
            loaded_mesh = loadMesh<VertexNormalTangent, GLuint>(assets, mesh, path, bones, bone_map, flags);
//        }

        meshes.emplace_back(loaded_mesh);
    }

    return meshes;
}

void ModelLoader::addAnimations(const fs::path& _path, const std::shared_ptr<AbstractModel>& skeletal) {
    auto& model = dynamic_cast<SkeletalModel&>(*skeletal);

    auto path = convertPathSeparators(_path);
    Assimp::Importer importer;
    const aiScene* scene;

    auto scene_flags = aiProcess_ValidateDataStructure |
                       aiProcess_Triangulate |
                       aiProcess_GenUVCoords |
                       aiProcess_GenNormals |
                       aiProcess_GenSmoothNormals |
                       aiProcess_CalcTangentSpace |
                       aiProcess_ImproveCacheLocality;

    scene = importer.ReadFile(path.string().c_str(), scene_flags);

    if (!scene || !scene->mRootNode) {
        throw model_loader_error(importer.GetErrorString());
    }

    auto& bone_map = model.getBoneMap();
    auto& bones = model.getBones();
    auto& animations = model.getAnimations();

    auto loaded = loadAnimations(scene, bones, bone_map, {});

    if (loaded.empty()) {
        throw model_loader_error{"Animations are empty!"};
    }

    animations.insert(animations.end(), std::make_move_iterator(loaded.begin()), std::make_move_iterator(loaded.end()));

    importer.FreeScene();
}

namespace Limitless {
    template std::vector<VertexNormalTangent> ModelLoader::loadVertices<VertexNormalTangent>(aiMesh* mesh, const ModelLoaderFlags& flags) noexcept;

    template std::vector<GLubyte> ModelLoader::loadIndices<GLubyte>(aiMesh* mesh) noexcept;
    template std::vector<GLushort> ModelLoader::loadIndices<GLushort>(aiMesh* mesh) noexcept;
    template std::vector<GLuint> ModelLoader::loadIndices<GLuint>(aiMesh* mesh) noexcept;
}
