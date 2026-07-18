#ifndef ANIMATION_H
#define ANIMATION_H

#include <windows.h>

#define DIR_COUNT 4

typedef struct Animation {
    HBITMAP image;

    int frameWidth;
    int frameHeight;
    int frameCount[DIR_COUNT];

    int currentFrame;
    int frameTimer;
    int frameDelay;
} Animation;

typedef struct NewAnimation {
    int frameWidth;
    int frameHeight;
    int frameCount[DIR_COUNT];

    int currentFrame;
    int frameTimer;
    int frameDelay;
} NewAnimation;

#endif