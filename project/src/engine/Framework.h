#pragma once

#include "BaseScene.h"
#include <memory>

class Framework {
public:
    virtual ~Framework();

    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual void Draw();
    virtual bool IsEndRequested();

    void Run();

protected:
    // Each concrete game Framework must supply the scene to start with.
    virtual std::unique_ptr<BaseScene> CreateInitialScene() = 0;
};