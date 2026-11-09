#include "AL3FirstFramework.h"
#include "AL3TitleScene.h"
#include <memory>

std::unique_ptr<BaseScene> AL3FirstFramework::CreateInitialScene() {
    return std::make_unique<AL3TitleScene>();
}