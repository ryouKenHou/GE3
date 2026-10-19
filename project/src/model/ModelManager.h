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

    // Loads a model from an .obj file (with its .mtl), or returns the cached one if already loaded
    const ModelData& Load(const std::string& directoryPath, const std::string& filename);

private:
    ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
    ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

    // Caches loaded models using "directoryPath/filename" as the key
    std::unordered_map<std::string, ModelData> models_;
};