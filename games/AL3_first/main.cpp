#include "AL3FirstFramework.h"
#include <windows.h>
#include <memory>

// ========================= Entry point =======================
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    auto game = std::make_unique<AL3FirstFramework>();

    game->Run();

    return 0;
}