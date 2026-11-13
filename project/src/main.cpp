#include <windows.h>
#include <memory>
#include "CG2Framework.h"

// ========================= Entry point =======================
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    auto game = std::make_unique<CG2Framework>();

    game->Run(1280,720,L"CG2 Framework");

    return 0;
}