#ifndef ANIMATION_H
#define ANIMATION_H

#include <windows.h>

#define DIR_COUNT 4

typedef struct Animation {
    HBITMAP image;

    float frameTimer;
    float frameDelay;

    int frameWidth;
    int frameHeight;
    int frameCount[DIR_COUNT];

    int currentFrame;
} Animation;

typedef struct NewAnimation {
    float frameTimer;
    float frameDelay;

    int frameWidth;
    int frameHeight;
    int frameCount[DIR_COUNT];

    int currentFrame;
} NewAnimation;

#endif