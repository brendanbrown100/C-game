#include "handler.h"
#include <string.h>
#include <stdio.h>

int Handler_Init(GameHandler *handler) {
    if (!Game_Init(&handler->game)) {
        return 0;
    }

    Home_Init(&handler->menu);
    Home_Refresh_Continue(handler);
    Pause_Init(&handler->pauseMenu);
    Settings_Init(&handler->settingsMenu);

    Load_Image(&handler->alphabetOrange, ALPHABET_ORANGE_PATH);
    Load_Image(&handler->alphabetRed, ALPHABET_RED_PATH);
    Load_Image(&handler->alphabetYellow, ALPHABET_YELLOW_PATH);
    Load_Image(&handler->alphabetPurple, ALPHABET_PURPLE_PATH);
    Load_Image(&handler->alphabetBlue, ALPHABET_BLUE_PATH);

    handler->currState = MENU;
    handler->game.lastFrameTimeDiff = 0.03125;
    handler->fps = 0;
    return 1;
}

int Handler_Update(GameHandler *handler) {
    switch (handler->currState) {
        case MENU: 
            if (!Home_Update(handler)) {
                printf("ERROR: Home_Update failed\n");
                return 0;
            }
            break;
        case PLAYING: 
            if (!Game_Update(handler)) {
                printf("ERROR: Game_Update failed\n");
                return 0;
            }
            break;
        case PAUSED:
            if (!Pause_Update(handler)) {
                printf("ERROR: Pause_Update failed\n");
                return 0;
            }
            break;
        case SETTINGS_STATE:
            if (!Settings_Update(handler)) {
                printf("ERROR: Settings_Update failed\n");
                return 0;
            }
            break;
    }
    return 1;
}

void Handler_Render(GameHandler *handler, HWND hwnd) {
    switch (handler->currState) {
        case MENU: 
            Home_Render(handler, hwnd);
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


void String_Render(GameHandler *handler, int startX, int startY, const char *text, int color, HDC hdc, HDC bufferDC) {
    HBITMAP alphabetBitmap = NULL;

    switch (color) {
        case STRING_BLUE:
            alphabetBitmap = handler->alphabetBlue;
            break;

        case STRING_RED:
            alphabetBitmap = handler->alphabetRed;
            break;
        
        case STRING_ORANGE:
            alphabetBitmap = handler->alphabetOrange;
            break;

        case STRING_YELLOW:
            alphabetBitmap = handler->alphabetYellow;
            break;

        case STRING_PURPLE:
            alphabetBitmap = handler->alphabetPurple;
            break;

        default:
            printf("ERROR: INVALID STRING COLOR\n");
            return;
    }

    if (text == NULL || alphabetBitmap == NULL) {
        return;
    }

    HDC alphabetDC = CreateCompatibleDC(hdc);

    if (alphabetDC == NULL) {
        return;
    }

    HBITMAP oldBitmap =
        (HBITMAP)SelectObject(alphabetDC, alphabetBitmap);

    int drawX = startX;
    int drawY = startY;

    for (int i = 0; text[i] != '\0'; i++) {
        unsigned char character = (unsigned char)text[i];

        if (character == '\n') {
            drawX = startX;
            drawY += ALPHABET_FRAME_HEIGHT;
            continue;
        }

        if (character == ' ') {
            drawX += ALPHABET_FRAME_WIDTH + ALPHABET_SPACING;
            continue;
        }

        character = (unsigned char)toupper(character);

        int frame;

        if (character >= '0' && character <= '9') {
            frame = character - '0';
        } else if (character >= 'A' && character <= 'Z') {
            frame = 10 + (character - 'A');
        } else {
            printf(
                "ERROR: '%c' CANNOT BE PRINTED\n",
                character
            );
            continue;
        }

        int srcX = frame * ALPHABET_FRAME_WIDTH;

        TransparentBlt(
            bufferDC,
            drawX,
            drawY,
            ALPHABET_FRAME_WIDTH,
            ALPHABET_FRAME_HEIGHT,
            alphabetDC,
            srcX,
            0,
            ALPHABET_FRAME_WIDTH,
            ALPHABET_FRAME_HEIGHT,
            RGB(0, 0, 0)
        );

        drawX += ALPHABET_FRAME_WIDTH + ALPHABET_SPACING;
    }

    SelectObject(alphabetDC, oldBitmap);
    DeleteDC(alphabetDC);
}

void Number_Render(GameHandler *handler, int startX, int startY, int num, int color, HDC hdc, HDC bufferDC) {
    char text[12];

    snprintf(text, sizeof(text), "%d", num);

    String_Render(
        handler,
        startX,
        startY,
        text,
        color,
        hdc,
        bufferDC
    );
}



