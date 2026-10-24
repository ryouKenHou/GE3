#include "SceneManager.h"

void SceneManager::ChangeScene(std::unique_ptr<BaseScene> nextScene) {
    nextScene_ = std::move(nextScene);
}

void SceneManager::Update() {
    // Perform scene transition if a new scene was requested
    if (nextScene_) {
        if (currentScene_) {
            currentScene_->Finalize();
        }
        currentScene_ = std::move(nextScene_);
        currentScene_->Initialize();
    }

    if (currentScene_) {
        currentScene_->Update();
    }
}

void SceneManager::Draw() {
    if (currentScene_) {
        currentScene_->Draw();
    }
}

void SceneManager::Finalize() {
    if (currentScene_) {
        currentScene_->Finalize();
        currentScene_.reset();
    }
}