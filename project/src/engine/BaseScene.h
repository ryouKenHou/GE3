#pragma once

class SceneManager;
class EngineCommon;

class BaseScene {
protected:
	EngineCommon* engineCommon_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
public:
    BaseScene() = default;
    virtual ~BaseScene() = default;

    virtual void Initialize(EngineCommon* engineCommon, SceneManager* sceneManager) = 0;
    virtual void Finalize() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
};