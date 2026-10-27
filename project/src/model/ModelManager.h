#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <sstream>
#include <cassert>
#include "EngineHelper.h" // ModelData, MaterialData, VertexData, Vector2/3/4

class ModelManager {
public:
    static ModelManager& GetInstance() {
        static ModelManager instance;
        return instance;
    }

    // Loads models (.obj, .gltf, .glb, .fbx, .dae, etc.)
    const ModelData& Load(const std::string& directoryPath, const std::string& filename);

private:
    ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    ModelData LoadModelWithAssimp(const std::string& fullPath, const std::string& directoryPath);

    std::unordered_map<std::string, ModelData> models_;
};