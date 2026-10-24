#include <windows.h>
#include "Framework.h"

// ========================= Entry point =======================
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    Framework* game = new Framework();

    game->Run();

    delete game;

    return 0;
}