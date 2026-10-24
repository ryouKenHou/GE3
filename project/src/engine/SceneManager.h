#pragma once
#include "BaseScene.h"
#include <memory>

class SceneManager {
public:
    static SceneManager& GetInstance() {
        static SceneManager instance;
        return instance;
    }

    void ChangeScene(std::unique_ptr<BaseScene> nextScene);
    void Update();
    void Draw();
    void Finalize();

private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unique_ptr<BaseScene> currentScene_ = nullptr;
    std::unique_ptr<BaseScene> nextScene_ = nullptr;
};