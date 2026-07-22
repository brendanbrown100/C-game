#include "handler.h"
#include "main.h"
#include "home.h"
#include "pause.h"
#include <stdio.h>

void Pause_Init(PauseMenu *pauseMenu) {
    pauseMenu->currSelected = RESUME;

    Load_Image(&pauseMenu->pausedTitle, PAUSED_TITLE_PATH);

    PauseOption resume = {0};
    resume.x = PAUSE_OPTIONS_X;
    resume.y = RESUME_OPTION_Y;
    resume.selected = 1;
    resume.type = RESUME;
    Load_Image(&resume.img, RESUME_OPTION_PATH);

    PauseOption homeOption = {0};
    homeOption.x = PAUSE_OPTIONS_X;
    homeOption.y = HOME_OPTION_Y;
    homeOption.selected = 0;
    homeOption.type = HOME;
    Load_Image(&homeOption.img, HOME_OPTION_PATH);

    PauseOption settings = {0};
    settings.x = PAUSE_OPTIONS_X;
    settings.y = SETTINGS_OPTION_Y;
    settings.selected = 0;
    settings.type = SETTINGS_PAUSE_OPTION;
    Load_Image(&settings.img, SETTINGS_OPTION_PATH);

    pauseMenu->options[RESUME] = resume;
    pauseMenu->options[HOME] = homeOption;
    pauseMenu->options[SETTINGS_PAUSE_OPTION] = settings;

    pauseMenu->player = 0;
    pauseMenu->upWasDown = 0;
    pauseMenu->downWasDown = 0;
    pauseMenu->selectWasDown = 1;
}

void Pause_Update(GameHandler *handler) {
    PauseMenu *pauseMenu = &handler->pauseMenu;

    int upIsDown = (GetAsyncKeyState(handler->game.playerKeyCodeData[pauseMenu->player].upKeyCode) & 0x8000) != 0;
    int downIsDown = (GetAsyncKeyState(handler->game.playerKeyCodeData[pauseMenu->player].downKeyCode) & 0x8000) != 0;
    int selectIsDown = (GetAsyncKeyState(handler->game.playerKeyCodeData[pauseMenu->player].selectKeyCode) & 0x8000) != 0;

    int upPressed = upIsDown && !pauseMenu->upWasDown;
    int downPressed = downIsDown && !pauseMenu->downWasDown;
    int selectPressed = selectIsDown && !pauseMenu->selectWasDown;

    if (upPressed) {
        pauseMenu->options[pauseMenu->currSelected].selected = 0;
        int next = pauseMenu->currSelected;
        next--;
        if (next < 0) next = PAUSE_OPTION_COUNT - 1;
        pauseMenu->options[next].selected = 1;
        pauseMenu->currSelected = next;
    } else if (downPressed) {
        pauseMenu->options[pauseMenu->currSelected].selected = 0;
        int next = pauseMenu->currSelected;
        next = (next + 1) % PAUSE_OPTION_COUNT;
        pauseMenu->options[next].selected = 1;
        pauseMenu->currSelected = next;
    }

    if (selectPressed) {
        PauseOptionType type = pauseMenu->options[pauseMenu->currSelected].type;

        switch (type) {
            case RESUME:
                handler->currState = PLAYING;
                break;
            case HOME:
                Game_Restart_Current_Level(&handler->game);
                Home_Refresh_Continue(handler);
                handler->currState = MENU;
                break;
            case SETTINGS_PAUSE_OPTION:
                handler->settingsMenu.playing = 1;
                handler->settingsMenu.player = pauseMenu->player;
                handler->currState = SETTINGS_STATE;
                break;
            case PAUSE_OPTION_COUNT:
                printf("This should not happen");
                break;

        }
    }

    pauseMenu->upWasDown = upIsDown;
    pauseMenu->downWasDown = downIsDown;
    pauseMenu->selectWasDown = selectIsDown;
}

void Pause_Render(PauseMenu *pauseMenu, HWND hwnd) {
    HDC hdc = GetDC(hwnd);

    HDC bufferDC = CreateCompatibleDC(hdc);
    HBITMAP bufferBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    HBITMAP oldBuffer = SelectObject(bufferDC, bufferBitmap);

    RECT screenRect = {0, 0, WIDTH, HEIGHT}; // x, y, width, height

    FillRect(bufferDC, &screenRect, (HBRUSH)(COLOR_WINDOW + 1)); // default background

    HDC pauseDC = CreateCompatibleDC(hdc);

    HBITMAP oldImage =
        SelectObject(pauseDC, pauseMenu->pausedTitle);

    TransparentBlt(
        bufferDC,
        PAUSED_TITLE_X,
        PAUSED_TITLE_Y,
        PAUSED_FRAME_WIDTH,
        PAUSED_FRAME_HEIGHT,
        pauseDC,
        0,
        0,
        PAUSED_FRAME_WIDTH,
        PAUSED_FRAME_HEIGHT,
        RGB(0, 0, 0)
    );

    SelectObject(pauseDC, oldImage);
    for (int i = 0; i < PAUSE_OPTION_COUNT; i++) {
        PauseOption *option = &pauseMenu->options[i];

        HBITMAP oldimage = SelectObject(pauseDC, option->img);

        int srcX = option->selected * PAUSE_OPTION_WIDTH;
        int srcY = 0;

        TransparentBlt(
            bufferDC, 
            option->x, 
            option->y, 
            PAUSE_OPTION_WIDTH, 
            PAUSE_OPTION_HEIGHT,
            pauseDC,
            srcX,
            srcY,
            PAUSE_OPTION_WIDTH,
            PAUSE_OPTION_HEIGHT,
            RGB(0, 0, 0)
        );
        SelectObject(pauseDC, oldimage);
    }
    BitBlt(
        hdc,
        0,
        0,
        WIDTH,
        HEIGHT,
        bufferDC,
        0,
        0,
        SRCCOPY
    );
    DeleteDC(pauseDC);

    SelectObject(bufferDC, oldBuffer);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDC);

    ReleaseDC(hwnd, hdc);
}