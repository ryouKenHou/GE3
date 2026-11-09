#pragma once
#include "Framework.h"

class CG2Framework : public Framework {
protected:
    std::unique_ptr<BaseScene> CreateInitialScene() override;
};