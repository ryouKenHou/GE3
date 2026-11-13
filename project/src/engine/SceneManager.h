#pragma once
#include <memory>

class BaseScene;
class EngineCommon;

class SceneManager {
public:
    //static SceneManager& GetInstance() {
    //    static SceneManager instance;
    //    return instance;
    //}
    SceneManager() = default;
    ~SceneManager() = default;

    void ChangeScene(std::unique_ptr<BaseScene> nextScene);
    void Update();
    void Draw();
    void Finalize();

    void SetEngineCommon(EngineCommon* engineCommon) { engineCommon_ = engineCommon; }

private:
    EngineCommon* engineCommon_ = nullptr;
    
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unique_ptr<BaseScene> currentScene_ = nullptr;
    std::unique_ptr<BaseScene> nextScene_ = nullptr;
};