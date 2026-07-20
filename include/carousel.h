#ifndef CAROUSEL_H
#define CAROUSEL_H

#include "animation.h"

#define CAROUSEL_PATH "Assets/Sprites/Objects/carousel.bmp"

#define CAROUSEL_FRAMES 18
#define CAROUSEL_FRAME_WIDTH 208
#define CAROUSEL_FRAME_HEIGHT 208
#define CAROUSEL_FRAME_DELAY 3

#define CAROUSEL_CENTER_SIZE 32
#define CAROUSEL_ARM_LENGTH 71
#define CAROUSEL_ARM_THICKNESS 6

#define CAROUSEL_ROTATION_PER_FRAME 5.0f
#define CAROUSEL_START_ANGLE 45.0f
#define CAROUSEL_ARM_START 22.6f

#define CAROUSEL_ATTACK_DELAY 20

#define PI 3.14159265358979323846f

#define CAROUSEL_DAMAGE 20

typedef struct Game Game;

typedef struct Carousel {
    Animation anim;

    int x;
    int y;
    int damage;
    int attackDelay;
    
    int clockWise;
    int attackHit;

    int remove;
} Carousel;

void Carousel_Init(Game *game);
void Carousel_Update(Game *game);
void Carousel_Render(Game *game, HDC hdc, HDC bufferDC);

#endif