#pragma once
#include "Framework.h"

class AL3FirstFramework : public Framework {
protected:
    std::unique_ptr<BaseScene> CreateInitialScene() override;
};