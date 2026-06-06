#include "main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Window window;

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    if (!Window_Create(&window, hInstance, nCmdShow, width, height, GAME_TITLE))
    {
        return 0;
    }

    Game game;

    if (!Game_Init(&game))
    {
        return 0;
    }

    MSG msg = {0};

    while (1)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Game_Update(&game);
        Game_Render(&game, window.hwnd);
        

        Sleep(16); // ~60 FPS
    }
}