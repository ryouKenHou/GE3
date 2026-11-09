#include "CG2Framework.h"
#include "TitleScene.h"
#include "GamePlayScene.h"
#include <memory>

std::unique_ptr<BaseScene> CG2Framework::CreateInitialScene() {
    //return std::make_unique<TitleScene>();
    return std::make_unique<GamePlayScene>();
}