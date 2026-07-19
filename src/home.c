#include "handler.h"
#include "main.h"
#include <stdio.h>
#include "home.h"

void Home_Init(Menu *menu) {
    menu->currSelected = NEW_GAME;

    MenuOption newGame = {0};
    newGame.x = MENU_OPTION_X;
    newGame.y = NEW_GAME_Y;
    newGame.show = 1;
    newGame.selected = 1;
    newGame.type = NEW_GAME;
    Load_Image(&newGame.img, NEW_GAME_PATH);

    MenuOption continueOption = {0};
    continueOption.x = MENU_OPTION_X;
    continueOption.y = CONTINUE_Y;
    continueOption.show = 0;
    continueOption.selected = 0;
    continueOption.type = CONTINUE;
    Load_Image(&continueOption.img, CONTINUE_PATH);

    MenuOption settings = {0};
    settings.x = MENU_OPTION_X;
    settings.y = SETTINGS_Y;
    settings.show = 1;
    settings.selected = 0;
    settings.type = SETTINGS;
    Load_Image(&settings.img, SETTINGS_PATH);

    menu->options[NEW_GAME] = newGame;
    menu->options[CONTINUE] = continueOption;
    menu->options[SETTINGS] = settings;

    menu->upWasDown = 0;
    menu->downWasDown = 0;
    menu->selectWasDown = 1;
}
void Home_Update(GameHandler *handler) {
    Menu *menu = &handler->menu;

    int upIsDown = (GetAsyncKeyState(handler->game.upKeyCode) & 0x8000) != 0;
    int downIsDown = (GetAsyncKeyState(handler->game.downKeyCode) & 0x8000) != 0;
    int selectIsDown = (GetAsyncKeyState(handler->game.selectKeyCode) & 0x8000) != 0;

    /*
     * A press happens only when:
     * - the key is down now
     * - the key was not down last frame
     */
    int upPressed = upIsDown && !menu->upWasDown;
    int downPressed = downIsDown && !menu->downWasDown;
    int selectPressed = selectIsDown && !menu->selectWasDown;

    if (upPressed) {
        menu->options[menu->currSelected].selected = 0;
        int next = menu->currSelected;
        do {
            next--;
            if (next < 0) next = MENU_OPTION_COUNT - 1;
        } while (!menu->options[next].show);
        menu->options[next].selected = 1;
        menu->currSelected = next;
    } else if (downPressed) {
        menu->options[menu->currSelected].selected = 0;
        int next = menu->currSelected;

        do {
            next = (next + 1) % MENU_OPTION_COUNT;
        } while (!menu->options[next].show);
        menu->options[next].selected = 1;
        menu->currSelected = next;
    }

    if (selectPressed) {
        MenuOptionType type = menu->options[menu->currSelected].type;

        switch (type) {
            case NEW_GAME:
                if (Game_Start_New(&handler->game)) {
                    Clear_Game_Data();
                    Home_Refresh_Continue(handler);
                    handler->currState = PLAYING;
                }
                break;
            case CONTINUE:
                if (Load_Game_Data(&handler->game)) {
                    handler->currState = PLAYING;
                } else {
                    printf("Continue failed\n");
                    menu->options[CONTINUE].show = 0;

                    if (menu->currSelected == CONTINUE) {
                        menu->options[CONTINUE].selected = 0;
                        menu->currSelected = NEW_GAME;
                        menu->options[NEW_GAME].selected = 1;
                    }
                }
                break;
            case SETTINGS:
                handler->settingsMenu.playing = 0;
                handler->currState = SETTINGS_STATE;
                break;
            case MENU_OPTION_COUNT:
                printf("This should not happen");
                break;

        }
    }

    menu->upWasDown = upIsDown;
    menu->downWasDown = downIsDown;
    menu->selectWasDown = selectIsDown;
}


void Home_Render(Menu *menu, HWND hwnd) {
    HDC hdc = GetDC(hwnd);

    HDC bufferDC = CreateCompatibleDC(hdc);
    HBITMAP bufferBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    HBITMAP oldBuffer = SelectObject(bufferDC, bufferBitmap);

    RECT screenRect = {0, 0, WIDTH, HEIGHT}; // x, y, width, height

    FillRect(bufferDC, &screenRect, (HBRUSH)(COLOR_WINDOW + 1)); // default background

    HDC homeDC = CreateCompatibleDC(hdc);
    for (int i = 0; i < MENU_OPTION_COUNT; i++) {
        MenuOption *option = &menu->options[i];
        if (!option->show) continue;

        HBITMAP oldimage = SelectObject(homeDC, option->img);

        int srcX = option->selected * MENU_OPTION_WIDTH;
        int srcY = 0;

        TransparentBlt(
            bufferDC, 
            option->x, 
            option->y, 
            MENU_OPTION_WIDTH, 
            MENU_OPTION_HEIGHT,
            homeDC,
            srcX,
            srcY,
            MENU_OPTION_WIDTH,
            MENU_OPTION_HEIGHT,
            RGB(0, 0, 0)
        );
        SelectObject(homeDC, oldimage);
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
    DeleteDC(homeDC);


    
    SelectObject(bufferDC, oldBuffer);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDC);

    ReleaseDC(hwnd, hdc);
}

void Home_Refresh_Continue(GameHandler *handler) {
    Menu *menu = &handler->menu;

    FILE *file = fopen(GAME_STATE_PATH, "rb");

    if (file == NULL) {
        menu->options[CONTINUE].show = 0;
        return;
    }

    GameData state = {0};

    size_t itemsRead = fread(
        &state,
        sizeof state,
        1,
        file
    );

    fclose(file);

    if (itemsRead == 1 &&
        state.level >= 0 &&
        state.level < handler->game.levelCount) {

        menu->options[CONTINUE].show = 1;
    } else {
        menu->options[CONTINUE].show = 0;
    }
    
    if (!menu->options[CONTINUE].show &&
        menu->currSelected == CONTINUE) {

        menu->options[CONTINUE].selected = 0;
        menu->currSelected = NEW_GAME;
        menu->options[NEW_GAME].selected = 1;
    }
}