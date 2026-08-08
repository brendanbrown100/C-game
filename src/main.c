#include "main.h"
#include "game_time.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

static void do_sleep( clock_t wait );
static int Set_Display_Resolution(int width, int height, DEVMODE *previousMode);
static void Restore_Display_Resolution(DEVMODE *previousMode);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    Window window;
    DEVMODE previousMode;

    GameHandler *handler = NULL;

    int displayChanged = 0;
    int exitCode = 0;

    const int width = WIDTH;
    const int height = HEIGHT;

    displayChanged = Set_Display_Resolution(width, height, &previousMode);

    if (!displayChanged) {
        return 0;
    }

    if (!Window_Create(&window, hInstance, nCmdShow, width, height, GAME_TITLE)) {
        MessageBox(NULL, "ERROR: Window_Create Failed", "DEBUG", MB_OK);
        goto cleanup;
    }

    handler = calloc(1, sizeof(GameHandler));

    if (handler == NULL) {
        printf("ERROR: CALLOC FAILED\n");
        MessageBox(NULL, "ERROR: calloc failed", "DEBUG", MB_OK);
        goto cleanup;
    }

    printf("GameHandler: %llu bytes\n", (unsigned long long)sizeof(*handler));

    if (!Handler_Init(handler)) {
        MessageBox(NULL, "ERROR: Handler_Init Failed", "DEBUG", MB_OK);
        goto cleanup;
    }

    MSG msg = {0};

    while (1)
    {
        clock_t start, final;
        double duration;

        start = clock();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) {
                exitCode = (int)msg.wParam;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x0001) {
            PostMessage(window.hwnd, WM_CLOSE, 0, 0);
        }

        if (!Handler_Update(handler)) {
            MessageBox(NULL, "ERROR: Handler_Update Failed", "DEBUG", MB_OK);
            goto cleanup;
        }
        Handler_Render(handler, window.hwnd);
        
        clock_t finish;
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        if (duration < TIME_PER_FRAME) {
            do_sleep((clock_t)((TIME_PER_FRAME - duration) * CLOCKS_PER_SEC));
        }
        else {
            printf("WARNING: GAME UPDATE TOO LONG FOR %d FPS - %.3f > %.3f\n", FPS, duration, TIME_PER_FRAME);
        }

        final = clock();
        duration = (double)(final - start) / CLOCKS_PER_SEC;
        handler->fps = 1.0 / duration;
        handler->game.deltaTime = (duration > TIME_PER_FRAME) ? TIME_PER_FRAME : duration;
        if (handler->currState == PLAYING) handler->game.time += duration;
    }
cleanup:
    free(handler);
    if (displayChanged) {
        Restore_Display_Resolution(&previousMode);
    }
    return exitCode;
}

static void do_sleep( clock_t wait ) {
   clock_t goal;
   goal = wait + clock();
   while( goal > clock() )
      ;
}

static int Set_Display_Resolution(int width, int height, DEVMODE *previousMode) {
    ZeroMemory(previousMode, sizeof(*previousMode));
    previousMode->dmSize = sizeof(*previousMode);

    /* Save the current display settings. */
    if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, previousMode)) {
        printf("ERROR: COULD NOT GET DISPLAY SETTINGS\n");
        return 0;
    }

    DEVMODE newMode = *previousMode;

    newMode.dmPelsWidth = width;
    newMode.dmPelsHeight = height;
    newMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

    /* Check whether Windows supports this resolution. */
    LONG result = ChangeDisplaySettings(&newMode, CDS_TEST);

    if (result != DISP_CHANGE_SUCCESSFUL) {
        printf("ERROR: %d X %d RESOLUTION NOT SUPPORTED\n", width, height);
        return 0;
    }

    /* Temporarily change the display resolution. */
    result = ChangeDisplaySettings(&newMode, CDS_FULLSCREEN);

    if (result != DISP_CHANGE_SUCCESSFUL) {
        printf("ERROR: COULD NOT CHANGE DISPLAY RESOLUTION\n");
        return 0;
    }

    return 1;
}

static void Restore_Display_Resolution(DEVMODE *previousMode) {
    ChangeDisplaySettings(previousMode, 0);
}