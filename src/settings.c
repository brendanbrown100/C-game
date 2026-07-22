#include "home.h"
#include "main.h"
#include "handler.h"
#include "settings.h"
#include <stdio.h>
#include <windows.h>

static int Save_Key_Codes(Game *game);
static int Get_KeyCode(Game *game, int type, int pIndex);

void Settings_Init(Settings *settings) {
    for (int i = 0; i < TOTAL_SETTINGS_OPTIONS; i++) {
        SettingsOption *option = &settings->options[i];

        option->type = i;
        option->x = SETTINGS_OPTION_X;
        option->y = SETTINGS_OPTION_START_Y + (SETTINGS_OPTION_INCREMENT_Y * i);
        option->selected = 0;
        option->changingKeyState = 0;
        option->remapDelay = 30;
    }
    settings->options[0].selected = 1;
    Load_Image(&settings->options[UP_KEY_OPTION].optionImg, UP_KEY_IMG_PATH);
    Load_Image(&settings->options[DOWN_KEY_OPTION].optionImg, DOWN_KEY_IMG_PATH); 
    Load_Image(&settings->options[LEFT_KEY_OPTION].optionImg, LEFT_KEY_IMG_PATH); 
    Load_Image(&settings->options[RIGHT_KEY_OPTION].optionImg, RIGHT_KEY_IMG_PATH); 
    Load_Image(&settings->options[SPRINT_KEY_OPTION].optionImg, SPRINT_KEY_IMG_PATH); 
    Load_Image(&settings->options[DASH_KEY_OPTION].optionImg, DASH_KEY_IMG_PATH); 
    Load_Image(&settings->options[ATTACK_KEY_OPTION].optionImg, ATTACK_KEY_IMG_PATH); 
    Load_Image(&settings->options[INTERACT_KEY_OPTION].optionImg, INTERACT_KEY_IMG_PATH); 
    Load_Image(&settings->options[SELECT_KEY_OPTION].optionImg, SELECT_KEY_IMG_PATH); 
    Load_Image(&settings->options[PAUSE_KEY_OPTION].optionImg, PAUSE_KEY_IMG_PATH);
    Load_Image(&settings->options[NUM_PLAYERS_OPTION].optionImg, NUM_PLAYERS_IMG_PATH);
    Load_Image(&settings->options[DAMPING_OPTION].optionImg, DAMPING_IMG_PATH);
    Load_Image(&settings->options[BACK_OPTION].optionImg, BACK_IMG_PATH); 
    Load_Image(&settings->settingsTitle, SETTINGS_TITLE_PATH); 

    settings->playing = 0;
    settings->player = 0;
    settings->upWasDown = 0;
    settings->downWasDown = 0;
    settings->selectWasDown = 1;
    settings->currSelected = 0;
}


void Settings_Update(GameHandler *handler) {
    Settings *settings = &handler->settingsMenu;
    Game *game = &handler->game;

    /*
     * Check whether an option is currently waiting for a new input.
     */
    for (int i = 0; i < TOTAL_SETTINGS_OPTIONS; i++) {
        SettingsOption *option = &settings->options[i];

        if (!option->changingKeyState) {
            continue;
        }

        /*
         * Wait a few frames so the Select button used to begin
         * remapping is not immediately captured.
         *
         * Do not call GetAsyncKeyState on every key during this delay,
         * because doing so can consume controller press events.
         */
        if (option->remapDelay > 0) {
            option->remapDelay--;
            return;
        }

        /*
         * This is deliberately the same input test that works in
         * your standalone input-testing program.
         */
        for (int virtualCode = 1;
             virtualCode < 256;
             virtualCode++) {

            SHORT keyState = GetAsyncKeyState(virtualCode);

            if (!(keyState & 0x8000)) {
                continue;
            }

            printf(
                "Detected input: %d | Hex: 0x%02X\n",
                virtualCode,
                virtualCode
            );

            switch (option->type) {
                case UP_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].upKeyCode = virtualCode;
                    break;

                case DOWN_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].downKeyCode = virtualCode;
                    break;

                case LEFT_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].leftKeyCode = virtualCode;
                    break;

                case RIGHT_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].rightKeyCode = virtualCode;
                    break;

                case SPRINT_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].sprintKeyCode = virtualCode;
                    break;

                case DASH_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].dashKeyCode = virtualCode;
                    break;

                case ATTACK_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].attackKeyCode = virtualCode;
                    break;

                case INTERACT_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].interactKeyCode = virtualCode;
                    break;

                case SELECT_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].selectKeyCode = virtualCode;
                    break;

                case PAUSE_KEY_OPTION:
                    game->playerKeyCodeData[settings->player].pauseKeyCode = virtualCode;
                    break;
                
                case NUM_PLAYERS_OPTION:
                    break;

                case DAMPING_OPTION:
                    break;

                case BACK_OPTION:
                    break;
            }

            printf(
                "Mapped option %d to input %d\n",
                option->type,
                virtualCode
            );


            option->changingKeyState = 0;
            option->remapDelay = 0;

            Save_Key_Codes(game);

            /*
             * Prevent the newly assigned input from immediately
             * activating the menu.
             */
            settings->upWasDown =
                (GetAsyncKeyState(game->playerKeyCodeData[settings->player].upKeyCode) &
                 0x8000) != 0;

            settings->downWasDown =
                (GetAsyncKeyState(game->playerKeyCodeData[settings->player].downKeyCode) &
                 0x8000) != 0;

            settings->selectWasDown =
                (GetAsyncKeyState(game->playerKeyCodeData[settings->player].selectKeyCode) &
                 0x8000) != 0;

            return;
        }

        /*
         * No new input detected yet.
         */
        return;
    }

    /*
     * Normal settings-menu controls.
     */
    int upIsDown =
        (GetAsyncKeyState(game->playerKeyCodeData[settings->player].upKeyCode) &
         0x8000) != 0;

    int downIsDown =
        (GetAsyncKeyState(game->playerKeyCodeData[settings->player].downKeyCode) &
         0x8000) != 0;

    int selectIsDown =
        (GetAsyncKeyState(game->playerKeyCodeData[settings->player].selectKeyCode) &
         0x8000) != 0;

    int upPressed =
        upIsDown && !settings->upWasDown;

    int downPressed =
        downIsDown && !settings->downWasDown;

    int selectPressed =
        selectIsDown && !settings->selectWasDown;

    if (upPressed) {
        settings->options[settings->currSelected].selected = 0;
        
        settings->currSelected--;

        if (settings->currSelected < 0) {
            settings->currSelected = TOTAL_SETTINGS_OPTIONS - 1;
        }

        settings->options[settings->currSelected].selected = 1;
    }
    else if (downPressed) {
        settings->options[settings->currSelected].selected = 0;

        settings->currSelected =
            (settings->currSelected + 1) %
            TOTAL_SETTINGS_OPTIONS;

        settings->options[settings->currSelected].selected = 1;
    }

    if (selectPressed) {
        SettingsOption *option =
            &settings->options[settings->currSelected];

        
        switch (option->type) {
            case BACK_OPTION:
                if (settings->playing) {
                    handler->currState = PAUSED;
                }
                else {
                    handler->currState = MENU;
                }

                settings->playing = 0;
                settings->selectWasDown = 1;
                return;
            case NUM_PLAYERS_OPTION:
                if (settings->player != 0) goto skip_num_player;
                int numPlayers = handler->game.numPlayers;
                numPlayers++;
                if (numPlayers > MAX_PLAYERS) numPlayers = 1;
                handler->game.numPlayers = numPlayers;
                skip_num_player:
                settings->upWasDown = upIsDown;
                settings->downWasDown = downIsDown;
                settings->selectWasDown = selectIsDown;
                return;
            case DAMPING_OPTION:
                if (settings->player != 0) goto skip_damping;
                int damping = (int)(handler->game.camera.damping * 10.0f);

                damping++;

                if (damping > 10) {
                    damping = 1;
                }

                handler->game.camera.damping = damping / 10.0f;

                printf(
                    "damping: %.1f\n",
                    handler->game.camera.damping
                );

                skip_damping:

                settings->upWasDown = upIsDown;
                settings->downWasDown = downIsDown;
                settings->selectWasDown = selectIsDown;
                return;
            
            default: 
                break;
                
        }
        
        option->changingKeyState = 1;
        option->remapDelay = 30;

        settings->upWasDown = upIsDown;
        settings->downWasDown = downIsDown;
        settings->selectWasDown = selectIsDown;

        printf(
            "Waiting for input for option %d...\n",
            option->type
        );

        return;
    }

    settings->upWasDown = upIsDown;
    settings->downWasDown = downIsDown;
    settings->selectWasDown = selectIsDown;
}

void Settings_Render(GameHandler *handler, HWND hwnd) {
    Settings *settings = &handler->settingsMenu;
    HDC hdc = GetDC(hwnd);

    HDC bufferDC = CreateCompatibleDC(hdc);
    HBITMAP bufferBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    HBITMAP oldBuffer = SelectObject(bufferDC, bufferBitmap);

    RECT screenRect = {0, 0, WIDTH, HEIGHT}; // x, y, width, height

    FillRect(bufferDC, &screenRect, (HBRUSH)(COLOR_WINDOW + 1)); // default background

    HDC settingsDC = CreateCompatibleDC(hdc);

    HBITMAP oldImage =
        SelectObject(settingsDC, settings->settingsTitle);

    TransparentBlt(
        bufferDC,
        SETTINGS_TITLE_X,
        SETTINGS_TITLE_Y,
        SETTINGS_TITLE_WIDTH,
        SETTINGS_TITLE_HEIGHT,
        settingsDC,
        0,
        0,
        SETTINGS_TITLE_WIDTH,
        SETTINGS_TITLE_HEIGHT,
        RGB(0, 0, 0)
    );

    SelectObject(settingsDC, oldImage);
    for (int i = 0; i < TOTAL_SETTINGS_OPTIONS; i++) {
        SettingsOption *option = &settings->options[i];

        HBITMAP oldimage = SelectObject(settingsDC, option->optionImg);

        int srcX = option->selected * SETTINGS_OPTION_WIDTH + (SETTINGS_OPTION_WIDTH * option->changingKeyState);
        int srcY = 0;

        TransparentBlt(
            bufferDC, 
            option->x, 
            option->y, 
            SETTINGS_OPTION_WIDTH, 
            SETTINGS_OPTION_HEIGHT,
            settingsDC,
            srcX,
            srcY,
            SETTINGS_OPTION_WIDTH,
            SETTINGS_OPTION_HEIGHT,
            RGB(0, 0, 0)
        );

        
        SelectObject(settingsDC, oldimage);

        int value = Get_KeyCode(&handler->game, option->type, settings->player);
        if (value) Number_Render(&handler->game, KEY_CODE_VAL_X, option->y, value, hdc, bufferDC);
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
    DeleteDC(settingsDC);

    Number_Render(&handler->game, FPS_X, FPS_Y, (int)handler->fps, hdc, bufferDC);
    Number_Render(&handler->game, 750, 50, (int) handler->settingsMenu.player + 1, hdc, bufferDC);
    SelectObject(bufferDC, oldBuffer);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDC);

    ReleaseDC(hwnd, hdc);
}

static int Get_KeyCode(Game *game, int type, int pIndex) {
    switch (type) {
        case (UP_KEY_OPTION):       return game->playerKeyCodeData[pIndex].upKeyCode;
        case (DOWN_KEY_OPTION):     return game->playerKeyCodeData[pIndex].downKeyCode;
        case (LEFT_KEY_OPTION):     return game->playerKeyCodeData[pIndex].leftKeyCode;
        case (RIGHT_KEY_OPTION):    return game->playerKeyCodeData[pIndex].rightKeyCode;
        case (SPRINT_KEY_OPTION):   return game->playerKeyCodeData[pIndex].sprintKeyCode;
        case (DASH_KEY_OPTION):     return game->playerKeyCodeData[pIndex].dashKeyCode;
        case (ATTACK_KEY_OPTION):   return game->playerKeyCodeData[pIndex].attackKeyCode;
        case (INTERACT_KEY_OPTION): return game->playerKeyCodeData[pIndex].interactKeyCode;
        case (SELECT_KEY_OPTION):   return game->playerKeyCodeData[pIndex].selectKeyCode;
        case (PAUSE_KEY_OPTION):    return game->playerKeyCodeData[pIndex].pauseKeyCode;
        case (NUM_PLAYERS_OPTION):  return game->numPlayers;
        case (DAMPING_OPTION):      return (int) (game->camera.damping * 10);
        case (BACK_OPTION):         return 0;
        default:                    return 0;
    }
}




static int Save_Key_Codes(Game *game) {
    KeyCodeData data = {0};

    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerKeyCodeData *playerKeyCodeData = &data.playerKeyCodeData[i];

        playerKeyCodeData->upKeyCode = game->playerKeyCodeData[i].upKeyCode;
        playerKeyCodeData->leftKeyCode = game->playerKeyCodeData[i].leftKeyCode;
        playerKeyCodeData->rightKeyCode = game->playerKeyCodeData[i].rightKeyCode;
        playerKeyCodeData->downKeyCode = game->playerKeyCodeData[i].downKeyCode;
        playerKeyCodeData->sprintKeyCode = game->playerKeyCodeData[i].sprintKeyCode;
        playerKeyCodeData->dashKeyCode = game->playerKeyCodeData[i].dashKeyCode;
        playerKeyCodeData->attackKeyCode = game->playerKeyCodeData[i].attackKeyCode;
        playerKeyCodeData->interactKeyCode = game->playerKeyCodeData[i].interactKeyCode;
        playerKeyCodeData->selectKeyCode = game->playerKeyCodeData[i].selectKeyCode;
        playerKeyCodeData->pauseKeyCode = game->playerKeyCodeData[i].pauseKeyCode;
    }

    FILE *file = fopen(GAME_KEY_CODES_PATH, "wb");

    if (file == NULL) {
        printf("Failed to open key codes file\n");
        return 0;
    }

    size_t written = fwrite(
        &data,
        sizeof data,
        1,
        file
    );

    fclose(file);

    if (written != 1) {
        printf("Failed to write key codes data\n");
        return 0;
    }

    return 1;
}
