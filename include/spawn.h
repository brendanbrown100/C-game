#ifndef SPAWN_H
#define SPAWN_H

#include <windows.h>
#include "animation.h"

#define HEALTH_BOX_PATH   "Assets/Sprites/Static/HealthBox.bmp"
#define STRENGTH_BOX_PATH "Assets/Sprites/Static/StrengthBox.bmp"

#define SPAWN_BOX_FRAME_WIDTH 32
#define SPAWN_BOX_FRAME_HEIGHT 32
#define SPAWN_BOX_FRAMES 4
#define SPAWN_BOX_FRAME_DELAY 10

#define HEALTH_BOX_AMOUNT 40
#define STRENGTH_BOX_AMOUNT 10


typedef enum SpawnType {
    HEALTH_BOX,
    STRENGTH_BOX,
    BOX_COUNT,
} SpawnType;


typedef struct Spawn {
    SpawnType type;
    Animation anim;
    
    int x;
    int y;

    int remove;
} Spawn;

#endif