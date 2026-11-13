#pragma once
#include "BaseScene.h"

class TitleScene : public BaseScene {
public:
    void Initialize(EngineCommon* engineCommon, SceneManager* sceneManager) override;
    void Finalize() override;
    void Update() override;
    void Draw() override;
};