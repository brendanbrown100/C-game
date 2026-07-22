#include "main.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static void do_sleep( clock_t wait );


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

    GameHandler *handler = (GameHandler*) calloc(1, sizeof(GameHandler));
    if (handler == NULL) {
        printf("ERROR: MALLOC FAILED\n");
        return 0;
    }

    if (!Handler_Init(handler))
    {
        return 0;
    }

    MSG msg = {0};

    while (1)
    {
        clock_t start, finish, final;
        double duration;
        double waitTime;

        start = clock();

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

        Handler_Update(handler);
        Handler_Render(handler, window.hwnd);
        

        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        if (duration < TIME_PER_FRAME) {
            waitTime = TIME_PER_FRAME - duration;
            do_sleep((clock_t)(waitTime * CLOCKS_PER_SEC));
        }
        else {
            printf("ERROR: GAME UPDATE TOO LONG FOR %d FPS - %.3f > %.3f\n", FPS, duration, TIME_PER_FRAME);
        }

    
        final = clock();
        duration = (double)(final - start) / CLOCKS_PER_SEC;
        handler->fps = 1.0 / duration;
        handler->game.time += duration;
    }
    free(handler);
    return (int)msg.wParam;
}

static void do_sleep( clock_t wait )
{
   clock_t goal;
   goal = wait + clock();
   while( goal > clock() )
      ;
}