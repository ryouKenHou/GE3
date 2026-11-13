#pragma once

#include "BaseScene.h"
#include "EngineCommon.h"
#include "SceneManager.h"
#include <memory>

class Framework {
public:
    virtual ~Framework();

    virtual void Initialize(int32_t Width, int32_t Height, const wchar_t* title);
    virtual void Finalize();
    virtual void Update();
    virtual void Draw();
    virtual bool IsEndRequested();

    void Run(int32_t Width, int32_t Height, const wchar_t* title);

protected:
    // Each concrete game Framework must supply the scene to start with.
    virtual std::unique_ptr<BaseScene> CreateInitialScene() = 0;

	EngineCommon engineCommon_;
	SceneManager sceneManager_;
};