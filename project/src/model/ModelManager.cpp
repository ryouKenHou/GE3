#include "ModelManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

const ModelData& ModelManager::Load(const std::string& directoryPath, const std::string& filename) {
    std::string key = directoryPath + "/" + filename;

    auto it = models_.find(key);
    if (it != models_.end()) {
        return it->second;
    }

    models_[key] = LoadModelWithAssimp(key, directoryPath);
    return models_[key];
}

ModelData ModelManager::LoadModelWithAssimp(const std::string& fullPath, const std::string& directoryPath) {
    Assimp::Importer importer;

    // Post-processing flags:
    // - aiProcess_Triangulate: Converts quads & n-gons into triangles
    // - aiProcess_FlipUVs: Flips UV texture y-coordinates (1.0 - y)
    // - aiProcess_GenSmoothNormals: Generates normals if missing
    // - aiProcess_CalcTangentSpace: Computes tangents/bitangents if needed
    uint32_t flags = aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices;

    const aiScene* scene = importer.ReadFile(fullPath, flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        assert(false && "Failed to load model using Assimp!");
        return ModelData{};
    }

    ModelData modelData;

    // Process all meshes in the scene
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh* mesh = scene->mMeshes[m];

        // Process material texture if available
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString texturePath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                modelData.material.textureFilePath = directoryPath + "/" + texturePath.C_Str();
            }
        }

        // Iterate through all faces (guaranteed to be triangles by aiProcess_Triangulate)
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
            aiFace face = mesh->mFaces[f];

            // Extract the 3 vertices for each triangle
            VertexData triangle[3];
            for (unsigned int i = 0; i < face.mNumIndices; ++i) {
                uint32_t index = face.mIndices[i];

                // Position
                aiVector3D pos = mesh->mVertices[index];
                Vector4 position = { pos.x, pos.y, pos.z, 1.0f };

                // Normal
                Vector3 normal = { 0.0f, 0.0f, 0.0f };
                if (mesh->HasNormals()) {
                    aiVector3D norm = mesh->mNormals[index];
                    normal = { norm.x, norm.y, norm.z };
                }

                // UV Coordinates
                Vector2 texcoord = { 0.0f, 0.0f };
                if (mesh->HasTextureCoords(0)) {
                    aiVector3D tex = mesh->mTextureCoords[0][index];
                    texcoord = { tex.x, tex.y };
                }

                // Match your coordinate system preferences (DirectX vs OpenGL conversions)
                position.x *= -1.0f;
                normal.x *= -1.0f;

                triangle[i] = { position, texcoord, normal };
            }

            // Maintain your winding order (reversing 0, 1, 2 to 2, 1, 0)
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        }
    }

    return modelData;
}