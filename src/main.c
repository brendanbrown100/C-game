#include "main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    Window window;

    int width = WIDTH;
    int height = HEIGHT;

    if (!Window_Create(&window, hInstance, nCmdShow, width, height, GAME_TITLE))
    {
        return 0;
    }

    GameHandler handler = {0};

    if (!Handler_Init(&handler))
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
        if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) {
            PostMessage(window.hwnd, WM_CLOSE, 0, 0);
        }
        Handler_Update(&handler);
        Handler_Render(&handler, window.hwnd);
        

        Sleep(16); // ~60 FPS
    }
    return (int)msg.wParam;
}