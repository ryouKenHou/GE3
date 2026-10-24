#pragma once

#include "BaseScene.h"
#include "EngineCommon.h"
#include "Object3D.hpp"
#include "Sprite.h"
#include "particleManager.h"
#include <memory>
#include <chrono>

class GamePlayScene : public BaseScene {
public:
    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

private:
    EngineCommon* engineCommon_ = nullptr;
    std::unique_ptr<SpriteCommon> spriteCommon_;

    int32_t kClientWidth_ = 0;
    int32_t kClientHeight_ = 0;

    std::unique_ptr<DebugCamera> debugCamera_;

    Transform transform_{ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {0.0f,0.0f,0.0f} };
    Transform transformball_{ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {0.0f,0.0f,0.0f} };
    Transform transformSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

    Transform uvTransform_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
    Transform uvTransformBall_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
    Transform uvTransformSprite_{ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };

    bool showPlane_ = true;
    bool showSprite_ = true;
    bool showBall_ = false;

    int frameCount_ = 0;

    std::unique_ptr<Model> model_;
    std::unique_ptr<Object3D> a_;
    std::unique_ptr<Sprite> b_;
    std::unique_ptr<Model> ball_;

    std::chrono::steady_clock::time_point lastFrameTime_;
    float deltaTime_ = 0.0f;
};