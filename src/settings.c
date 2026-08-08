#include "home.h"
#include "main.h"
#include "handler.h"
#include "settings.h"
#include <stdio.h>
#include <windows.h>

static int Save_Key_Codes(Game *game);
static const char *Get_KeyCode(Game *game, int type, int pIndex);
const char *KeyCode_To_String(int keyCode);

static const char *SETTINGS_OPTION_TEXT[] = {
    "PLAYER",
    "MOVE UP",
    "MOVE DOWN",
    "MOVE LEFT",
    "MOVE RIGHT",
    "SPRINT",
    "DASH",
    "ATTACK",
    "INTERACT",
    "SELECT",
    "PAUSE",
    "DAMPING",
    "NUMBER PLAYERS",
    "BACK"
};

static const int SETTINGS_OPTIONS_Y[] = {
    100, // PLAYER_OPTION
    150, // UP_KEY_OPTION
    180, // DOWN_KEY_OPTION
    210, // LEFT_KEY_OPTION
    240, // RIGHT_KEY_OPTION
    270, // SPRINT_KEY_OPTION
    300, // DASH_KEY_OPTION
    330, // ATTACK_KEY_OPTION
    360, // INTERACT_KEY_OPTION
    390, // SELECT_KEY_OPTION
    420, // PAUSE_KEY_OPTION
    470, // DAMPING_OPTION
    500, // NUM_PLAYERS_OPTION
    550  // BACK_OPTION
};


void Settings_Init(Settings *settings) {
    for (int i = 0; i < TOTAL_SETTINGS_OPTIONS; i++) {
        SettingsOption *option = &settings->options[i];

        option->type = i;
        option->x = SETTINGS_OPTION_X;
        option->y = SETTINGS_OPTIONS_Y[i];
        option->selected = 0;
        option->changingKeyState = 0;
        option->remapDelay = OPTION_REMAP_DELAY;
        option->show = 1;
    }
    settings->options[0].selected = 1;
    settings->playing = 0;
    settings->player = 0;
    settings->upWasDown = 0;
    settings->downWasDown = 0;
    settings->selectWasDown = 1;
    settings->currSelected = 0;
}


int Settings_Update(GameHandler *handler) {
    Settings *settings = &handler->settingsMenu;
    Game *game = &handler->game;
    
    settings->options[NUM_PLAYERS_OPTION].show = (settings->playing || settings->player != 0) ? 0 : 1;
    settings->options[DAMPING_OPTION].show = (settings->playing || settings->player != 0) ? 0 : 1;
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
            option->remapDelay -= handler->game.deltaTime;
            return 1;
        }

        /*
         * This is deliberately the same input test that works in
         * your standalone input-testing program.
         */
        for (int virtualCode = 1;
             virtualCode < 255;
             virtualCode++) {

            SHORT keyState = GetAsyncKeyState(virtualCode);

            if (!(keyState & 0x8000)) {
                continue;
            }

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
                
                default:
                    break;
            }


            option->changingKeyState = 0;
            option->remapDelay = 0.0f;

            if (!Save_Key_Codes(game)) return 0;

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

            return 1;
        }

        /*
         * No new input detected yet.
         */
        return 1;
    }

    /*
     * Normal settings-menu controls.
     */
    int upKeyCode = (game->playerKeyCodeData[settings->player].upKeyCode > 0) ? game->playerKeyCodeData[settings->player].upKeyCode : VK_UP;
    int upIsDown =
        (GetAsyncKeyState(upKeyCode) &
         0x8000) != 0;

    int downKeyCode = (game->playerKeyCodeData[settings->player].downKeyCode > 0) ? game->playerKeyCodeData[settings->player].downKeyCode : VK_DOWN;
    int downIsDown =
        (GetAsyncKeyState(downKeyCode) &
         0x8000) != 0;

    int selectKeyCode = (game->playerKeyCodeData[settings->player].selectKeyCode > 0) ? game->playerKeyCodeData[settings->player].selectKeyCode : VK_RETURN;
    int selectIsDown =
        (GetAsyncKeyState(selectKeyCode) &
         0x8000) != 0;

    int upPressed =
        upIsDown && !settings->upWasDown;

    int downPressed =
        downIsDown && !settings->downWasDown;

    int selectPressed =
        selectIsDown && !settings->selectWasDown;

    if (upPressed) {
        settings->options[settings->currSelected].selected = 0;
        int next = settings->currSelected;
        do {
            next--;
            if (next < 0) next = TOTAL_SETTINGS_OPTIONS - 1;
        } while (!settings->options[next].show);
        settings->options[next].selected = 1;
        settings->currSelected = next;
    }
    else if (downPressed) {
        settings->options[settings->currSelected].selected = 0;
        int next = settings->currSelected;
        do {
            next = (next + 1) % TOTAL_SETTINGS_OPTIONS;
        } while (!settings->options[next].show);
        settings->options[next].selected = 1;
        settings->currSelected = next;
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
                return 1;
            case NUM_PLAYERS_OPTION:
                int numPlayers = handler->game.numPlayers;
                numPlayers++;
                if (numPlayers > MAX_PLAYERS) numPlayers = 1;
                handler->game.numPlayers = numPlayers;
                settings->upWasDown = upIsDown;
                settings->downWasDown = downIsDown;
                settings->selectWasDown = selectIsDown;
                return 1;
            case DAMPING_OPTION:
                int damping = (int)(handler->game.camera.damping * 11.0f);

                damping++;

                if (damping > 10) {
                    damping = 0;
                }

                handler->game.camera.damping = damping / 11.0f;

                settings->upWasDown = upIsDown;
                settings->downWasDown = downIsDown;
                settings->selectWasDown = selectIsDown;
                return 1;
            
            case PLAYER_OPTION:
                int player = settings->player;
                player++;
                if (player >= handler->game.numPlayers) player = 0;
                settings->player = player;
                settings->upWasDown = upIsDown;
                settings->downWasDown = downIsDown;
                settings->selectWasDown = selectIsDown;
                return 1;
            
            default: 
                break;
                
        }
        
        option->changingKeyState = 1;
        option->remapDelay = OPTION_REMAP_DELAY;

        settings->upWasDown = upIsDown;
        settings->downWasDown = downIsDown;
        settings->selectWasDown = selectIsDown;

        return 1;
    }

    settings->upWasDown = upIsDown;
    settings->downWasDown = downIsDown;
    settings->selectWasDown = selectIsDown;
    return 1;
}

void Settings_Render(GameHandler *handler, HWND hwnd) {
    Settings *settings = &handler->settingsMenu;
    HDC hdc = GetDC(hwnd);

    HDC bufferDC = CreateCompatibleDC(hdc);
    HBITMAP bufferBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    HBITMAP oldBuffer = SelectObject(bufferDC, bufferBitmap);

    RECT screenRect = {0, 0, WIDTH, HEIGHT}; // x, y, width, height

    HBRUSH backgroundBrush = CreateSolidBrush(RGB(132, 126, 135));
    FillRect(
        bufferDC,
        &screenRect,
        backgroundBrush
    );
    DeleteObject(backgroundBrush);

    String_Render(
        handler,
        SETTINGS_TITLE_X,
        SETTINGS_TITLE_Y,
        "SETTINGS",
        STRING_YELLOW,
        hdc,
        bufferDC
    );
    // TransparentBlt(
    //     bufferDC,
    //     SETTINGS_TITLE_X,
    //     SETTINGS_TITLE_Y,
    //     SETTINGS_TITLE_WIDTH,
    //     SETTINGS_TITLE_HEIGHT,
    //     settingsDC,
    //     0,
    //     0,
    //     SETTINGS_TITLE_WIDTH,
    //     SETTINGS_TITLE_HEIGHT,
    //     RGB(0, 0, 0)
    // );

    for (int i = 0; i < TOTAL_SETTINGS_OPTIONS; i++) {
        SettingsOption *option = &settings->options[i];
        if (!option->show) continue;
        int color = (option->selected) ? STRING_RED : STRING_BLUE;
        if (color == STRING_RED) color = (option->changingKeyState) ? STRING_ORANGE : STRING_RED;

        String_Render(
            handler,
            option->x,
            option->y,
            SETTINGS_OPTION_TEXT[i],
            color,
            hdc,
            bufferDC
        );

        const char *text = Get_KeyCode(&handler->game, option->type, settings->player);
        String_Render(handler, KEY_CODE_VAL_X, option->y, text, STRING_PURPLE, hdc, bufferDC);
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

    SelectObject(bufferDC, oldBuffer);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDC);

    ReleaseDC(hwnd, hdc);
}

static const char *Get_KeyCode(Game *game, int type, int pIndex) {
    switch (type) {
        case (UP_KEY_OPTION):       return KeyCode_To_String(game->playerKeyCodeData[pIndex].upKeyCode);
        case (DOWN_KEY_OPTION):     return KeyCode_To_String(game->playerKeyCodeData[pIndex].downKeyCode);
        case (LEFT_KEY_OPTION):     return KeyCode_To_String(game->playerKeyCodeData[pIndex].leftKeyCode);
        case (RIGHT_KEY_OPTION):    return KeyCode_To_String(game->playerKeyCodeData[pIndex].rightKeyCode);
        case (SPRINT_KEY_OPTION):   return KeyCode_To_String(game->playerKeyCodeData[pIndex].sprintKeyCode);
        case (DASH_KEY_OPTION):     return KeyCode_To_String(game->playerKeyCodeData[pIndex].dashKeyCode);
        case (ATTACK_KEY_OPTION):   return KeyCode_To_String(game->playerKeyCodeData[pIndex].attackKeyCode);
        case (INTERACT_KEY_OPTION): return KeyCode_To_String(game->playerKeyCodeData[pIndex].interactKeyCode);
        case (SELECT_KEY_OPTION):   return KeyCode_To_String(game->playerKeyCodeData[pIndex].selectKeyCode);
        case (PAUSE_KEY_OPTION):    return KeyCode_To_String(game->playerKeyCodeData[pIndex].pauseKeyCode);
        
        static char text[12];
        case (NUM_PLAYERS_OPTION):  
            snprintf(text, sizeof(text), "%d", game->numPlayers);
            return text;
        case (PLAYER_OPTION):       
            snprintf(text, sizeof(text), "%d", pIndex + 1);    
            return text;
        case (DAMPING_OPTION):      
            snprintf(text, sizeof(text), "%d", (int) (game->camera.damping * 11.0f));
            return text;
        default: return "";
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
        printf("ERROR: FILE NOT FOUND - failed to open key codes file\n");
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
        printf("ERROR: FILE FORMAT - failed to write key codes data\n");
        return 0;
    }

    return 1;
}



const char *KeyCode_To_String(int keyCode)
{
    switch (keyCode) {
        /* Mouse buttons */
        case VK_LBUTTON:  return "LEFT MOUSE";
        case VK_RBUTTON:  return "RIGHT MOUSE";
        case VK_CANCEL:   return "CONTROL BREAK";
        case VK_MBUTTON:  return "MIDDLE MOUSE";
        case VK_XBUTTON1: return "MOUSE 4";
        case VK_XBUTTON2: return "MOUSE 5";

        /* General keyboard keys */
        case VK_BACK:     return "BACKSPACE";
        case VK_TAB:      return "TAB";
        case VK_CLEAR:    return "CLEAR";
        case VK_RETURN:   return "ENTER";
        case VK_SHIFT:    return "SHIFT";
        case VK_CONTROL:  return "CONTROL";
        case VK_MENU:     return "ALT";
        case VK_PAUSE:    return "PAUSE";
        case VK_CAPITAL:  return "CAPS LOCK";
        case VK_ESCAPE:   return "ESCAPE";
        case VK_SPACE:    return "SPACE";

        /* Navigation keys */
        case VK_PRIOR:    return "PAGE UP";
        case VK_NEXT:     return "PAGE DOWN";
        case VK_END:      return "END";
        case VK_HOME:     return "HOME";
        case VK_LEFT:     return "LEFT";
        case VK_UP:       return "UP";
        case VK_RIGHT:    return "RIGHT";
        case VK_DOWN:     return "DOWN";
        case VK_SELECT:   return "SELECT";
        case VK_PRINT:    return "PRINT";
        case VK_EXECUTE:  return "EXECUTE";
        case VK_SNAPSHOT: return "PRINT SCREEN";
        case VK_INSERT:   return "INSERT";
        case VK_DELETE:   return "DELETE";
        case VK_HELP:     return "HELP";

        /* Number keys */
        case 0x30: return "0";
        case 0x31: return "1";
        case 0x32: return "2";
        case 0x33: return "3";
        case 0x34: return "4";
        case 0x35: return "5";
        case 0x36: return "6";
        case 0x37: return "7";
        case 0x38: return "8";
        case 0x39: return "9";

        /* Letter keys */
        case 0x41: return "A";
        case 0x42: return "B";
        case 0x43: return "C";
        case 0x44: return "D";
        case 0x45: return "E";
        case 0x46: return "F";
        case 0x47: return "G";
        case 0x48: return "H";
        case 0x49: return "I";
        case 0x4A: return "J";
        case 0x4B: return "K";
        case 0x4C: return "L";
        case 0x4D: return "M";
        case 0x4E: return "N";
        case 0x4F: return "O";
        case 0x50: return "P";
        case 0x51: return "Q";
        case 0x52: return "R";
        case 0x53: return "S";
        case 0x54: return "T";
        case 0x55: return "U";
        case 0x56: return "V";
        case 0x57: return "W";
        case 0x58: return "X";
        case 0x59: return "Y";
        case 0x5A: return "Z";

        /* Windows keys */
        case VK_LWIN:  return "LEFT WINDOWS";
        case VK_RWIN:  return "RIGHT WINDOWS";
        case VK_APPS:  return "APPLICATION";
        case VK_SLEEP: return "SLEEP";

        /* Number pad */
        case VK_NUMPAD0:   return "NUMPAD 0";
        case VK_NUMPAD1:   return "NUMPAD 1";
        case VK_NUMPAD2:   return "NUMPAD 2";
        case VK_NUMPAD3:   return "NUMPAD 3";
        case VK_NUMPAD4:   return "NUMPAD 4";
        case VK_NUMPAD5:   return "NUMPAD 5";
        case VK_NUMPAD6:   return "NUMPAD 6";
        case VK_NUMPAD7:   return "NUMPAD 7";
        case VK_NUMPAD8:   return "NUMPAD 8";
        case VK_NUMPAD9:   return "NUMPAD 9";
        case VK_MULTIPLY:  return "NUMPAD MULTIPLY";
        case VK_ADD:       return "NUMPAD ADD";
        case VK_SEPARATOR: return "NUMPAD SEPARATOR";
        case VK_SUBTRACT:  return "NUMPAD SUBTRACT";
        case VK_DECIMAL:   return "NUMPAD DECIMAL";
        case VK_DIVIDE:    return "NUMPAD DIVIDE";

        /* Function keys */
        case VK_F1:  return "F1";
        case VK_F2:  return "F2";
        case VK_F3:  return "F3";
        case VK_F4:  return "F4";
        case VK_F5:  return "F5";
        case VK_F6:  return "F6";
        case VK_F7:  return "F7";
        case VK_F8:  return "F8";
        case VK_F9:  return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";
        case VK_F13: return "F13";
        case VK_F14: return "F14";
        case VK_F15: return "F15";
        case VK_F16: return "F16";
        case VK_F17: return "F17";
        case VK_F18: return "F18";
        case VK_F19: return "F19";
        case VK_F20: return "F20";
        case VK_F21: return "F21";
        case VK_F22: return "F22";
        case VK_F23: return "F23";
        case VK_F24: return "F24";

        case VK_NUMLOCK: return "NUM LOCK";
        case VK_SCROLL:  return "SCROLL LOCK";

        /* Left and right modifier keys */
        case VK_LSHIFT:   return "LEFT SHIFT";
        case VK_RSHIFT:   return "RIGHT SHIFT";
        case VK_LCONTROL: return "LEFT CONTROL";
        case VK_RCONTROL: return "RIGHT CONTROL";
        case VK_LMENU:    return "LEFT ALT";
        case VK_RMENU:    return "RIGHT ALT";

        /* Browser keys */
        case VK_BROWSER_BACK:      return "BROWSER BACK";
        case VK_BROWSER_FORWARD:   return "BROWSER FORWARD";
        case VK_BROWSER_REFRESH:   return "BROWSER REFRESH";
        case VK_BROWSER_STOP:      return "BROWSER STOP";
        case VK_BROWSER_SEARCH:    return "BROWSER SEARCH";
        case VK_BROWSER_FAVORITES: return "BROWSER FAVORITES";
        case VK_BROWSER_HOME:      return "BROWSER HOME";

        /* Volume and media keys */
        case VK_VOLUME_MUTE:         return "VOLUME MUTE";
        case VK_VOLUME_DOWN:         return "VOLUME DOWN";
        case VK_VOLUME_UP:           return "VOLUME UP";
        case VK_MEDIA_NEXT_TRACK:    return "NEXT TRACK";
        case VK_MEDIA_PREV_TRACK:    return "PREVIOUS TRACK";
        case VK_MEDIA_STOP:          return "MEDIA STOP";
        case VK_MEDIA_PLAY_PAUSE:    return "PLAY PAUSE";
        case VK_LAUNCH_MAIL:         return "MAIL";
        case VK_LAUNCH_MEDIA_SELECT: return "MEDIA SELECT";
        case VK_LAUNCH_APP1:         return "APPLICATION 1";
        case VK_LAUNCH_APP2:         return "APPLICATION 2";

        /* Punctuation keys */
        case VK_OEM_1:      return "SEMICOLON";
        case VK_OEM_PLUS:   return "EQUALS";
        case VK_OEM_COMMA:  return "COMMA";
        case VK_OEM_MINUS:  return "MINUS";
        case VK_OEM_PERIOD: return "PERIOD";
        case VK_OEM_2:      return "FORWARD SLASH";
        case VK_OEM_3:      return "GRAVE";
        case VK_OEM_4:      return "LEFT BRACKET";
        case VK_OEM_5:      return "BACKSLASH";
        case VK_OEM_6:      return "RIGHT BRACKET";
        case VK_OEM_7:      return "APOSTROPHE";
        case VK_OEM_8:      return "OEM 8";
        case VK_OEM_102:    return "OEM 102";

        /*
         * Gamepad virtual-key values.
         * Hex values are used in case your MinGW Windows headers do
         * not define the VK_GAMEPAD constants.
         */
        case 0xC3: return "GAMEPAD A";
        case 0xC4: return "GAMEPAD B";
        case 0xC5: return "GAMEPAD X";
        case 0xC6: return "GAMEPAD Y";
        case 0xC7: return "RIGHT SHOULDER";
        case 0xC8: return "LEFT SHOULDER";
        case 0xC9: return "LEFT TRIGGER";
        case 0xCA: return "RIGHT TRIGGER";
        case 0xCB: return "DPAD UP";
        case 0xCC: return "DPAD DOWN";
        case 0xCD: return "DPAD LEFT";
        case 0xCE: return "DPAD RIGHT";
        case 0xCF: return "GAMEPAD MENU";
        case 0xD0: return "GAMEPAD VIEW";
        case 0xD1: return "LEFT STICK BUTTON";
        case 0xD2: return "RIGHT STICK BUTTON";
        case 0xD3: return "LEFT STICK UP";
        case 0xD4: return "LEFT STICK DOWN";
        case 0xD5: return "LEFT STICK RIGHT";
        case 0xD6: return "LEFT STICK LEFT";
        case 0xD7: return "RIGHT STICK UP";
        case 0xD8: return "RIGHT STICK DOWN";
        case 0xD9: return "RIGHT STICK RIGHT";
        case 0xDA: return "RIGHT STICK LEFT";

        /* Less common keys */
        case VK_PROCESSKEY: return "PROCESS";
        case VK_PACKET:     return "PACKET";
        case VK_ATTN:       return "ATTENTION";
        case VK_CRSEL:      return "CRSEL";
        case VK_EXSEL:      return "EXSEL";
        case VK_EREOF:      return "ERASE EOF";
        case VK_PLAY:       return "PLAY";
        case VK_ZOOM:       return "ZOOM";
        case VK_NONAME:     return "NO NAME";
        case VK_PA1:        return "PA1";
        case VK_OEM_CLEAR:  return "OEM CLEAR";

        default:
            return "UNKNOWN";
    }
}