#pragma once

class Framework {
public:
    virtual ~Framework();

    virtual void Initialize();
    virtual void Finalize();
    virtual void Update();
    virtual void Draw();
    virtual bool IsEndRequested();

    void Run();
};