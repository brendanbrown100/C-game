#include "handler.h"
#include <string.h>

int Handler_Init(GameHandler *handler) {
    if (!Game_Init(&handler->game)) {
        return 0;
    }

    Home_Init(&handler->menu);
    Home_Refresh_Continue(handler);
    Pause_Init(&handler->pauseMenu);
    Settings_Init(&handler->settingsMenu);

    handler->currState = MENU;
    handler->fps = 0;
    return 1;
}

void Handler_Update(GameHandler *handler) {
    if (handler->currState == PLAYING &&
        (GetAsyncKeyState(handler->game.pauseKeyCode) & 0x0001)) {

        handler->currState = PAUSED;
        return;
    }
    switch (handler->currState) {
        case MENU: 
            Home_Update(handler);
            break;
        case PLAYING: 
            Game_Update(handler);
            break;
        case PAUSED:
            Pause_Update(handler);
            break;
        case SETTINGS_STATE:
            Settings_Update(handler);
            break;
    }
}

void Handler_Render(GameHandler *handler, HWND hwnd) {
    switch (handler->currState) {
        case MENU: 
            Home_Render(&handler->menu, hwnd);
            break;
        case PLAYING: 
            Game_Render(handler, hwnd);
            break;
        case PAUSED:
            Pause_Render(&handler->pauseMenu, hwnd);
            break;
        case SETTINGS_STATE:
            Settings_Render(handler, hwnd);
            break;
    }
}
