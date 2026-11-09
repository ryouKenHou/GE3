#pragma once
#include "BaseScene.h"
#include "EngineCommon.h"
#include "Object3D.hpp"
#include "Sprite.h"

class AL3TitleScene : public BaseScene {
    Transform transformSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
    EngineCommon* engineCommon_ = nullptr;
    std::unique_ptr<Sprite> b_;

    Transform uvTransformSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
    int32_t kClientWidth_ = 1280;
    int32_t kClientHeight_ =720;
    std::unique_ptr<SpriteCommon> spriteCommon_;
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;
};