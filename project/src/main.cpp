#include <windows.h>
#include <memory>
#include "Framework.h"

// ========================= Entry point =======================
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    auto game = std::make_unique<Framework>();

    game->Run();

    return 0;
}