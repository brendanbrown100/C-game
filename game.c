#include "game.h"
#include "main.h"
#include "player.h"
#include "enemy.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>

static void Game_UpdateCamera(Game *game);
static int Level_Won(Game *game);
static int Game_InitLevel(Game *game, const char *levelPath);
static void Game_Over(Game *game, HDC hdc, HDC bufferDC);
static void Load_Image(HBITMAP *bitmap, const char *path);



const char *levelPaths[] = {
    LEVEL_1_PATH,
    LEVEL_2_PATH
};


int Game_Init(Game *game) {
    if (game == NULL) return 0;
    srand((unsigned int) time(NULL));
    printf("%zu\n", sizeof(*game));
    

    game->levelCount = sizeof(levelPaths) / sizeof(levelPaths[0]);
    game->currentLevel = 0;
    game->gameOver = 0;
    game->gameWin = 0;

    Load_Image(&game->wallTile, WALL_TILE_PATH);
    Load_Image(&game->wallUpTile, WALL_UP_TILE_PATH);
    Load_Image(&game->wallDownTile, WALL_DOWN_TILE_PATH);
    Load_Image(&game->wallLeftTile, WALL_LEFT_TILE_PATH);
    Load_Image(&game->wallRightTile, WALL_RIGHT_TILE_PATH);
    Load_Image(&game->wallTLTile, WALL_TL_TILE_PATH);
    Load_Image(&game->wallTRTile, WALL_TR_TILE_PATH);
    Load_Image(&game->wallBLTile, WALL_BL_TILE_PATH);
    Load_Image(&game->wallBRTile, WALL_BR_TILE_PATH);
    Load_Image(&game->wallLUTile, WALL_LU_TILE_PATH);
    Load_Image(&game->wallRUTile, WALL_RU_TILE_PATH);
    Load_Image(&game->wallLDTile, WALL_LD_TILE_PATH);
    Load_Image(&game->wallRDTile, WALL_RD_TILE_PATH);
    Load_Image(&game->goalClosedTile, GOAL_CLOSED_TILE_PATH);
    Load_Image(&game->goalOpenTile, GOAL_OPEN_TILE_PATH);
    Load_Image(&game->floorTile, FLOOR_TILE_PATH);


    if (!Game_InitLevel(game, LEVEL_1_PATH)) {
        return 0;
    }

    Image_Init(
        &game->gameOverAnim,
        GAME_OVER_PATH,
        GAME_OVER_FRAME_WIDTH,
        GAME_OVER_FRAME_HEIGHT,
        GAME_OVER_FRAME_DELAY,
        (int[]){GAME_OVER_FRAMES, GAME_OVER_FRAMES, GAME_OVER_FRAMES, GAME_OVER_FRAMES}
    );

    Image_Init(
        &game->spawnAnim[HEALTH_BOX],
        HEALTH_BOX_PATH,
        SPAWN_BOX_FRAME_WIDTH,
        SPAWN_BOX_FRAME_HEIGHT,
        SPAWN_BOX_FRAME_DELAY,
        (int[]){SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES}
    );

    Image_Init(
        &game->spawnAnim[STRENGTH_BOX],
        STRENGTH_BOX_PATH,
        SPAWN_BOX_FRAME_WIDTH,
        SPAWN_BOX_FRAME_HEIGHT,
        SPAWN_BOX_FRAME_DELAY,
        (int[]){SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES}
    );

    Image_Init(
        &game->coinAnim,
        COIN_PATH,
        COIN_FRAME_WIDTH,
        COIN_FRAME_HEIGHT,
        COIN_FRAME_DELAY,
        (int[]){COIN_FRAMES, COIN_FRAMES, COIN_FRAMES, COIN_FRAMES}
    );



    game->camera.width = GetSystemMetrics(SM_CXSCREEN);
    game->camera.height = GetSystemMetrics(SM_CYSCREEN);
    game->camera.shakeTimer = 0;
    game->camera.shakeDuration = 0;
    game->camera.shakeStrength = 0;
    game->camera.shakeOffsetX = 0;
    game->camera.shakeOffsetY = 0;

    return 1;

}

static int Game_InitLevel(Game *game, const char *levelPath) {
    Level *level = &game->levels[game->currentLevel];

    
    level->spawnCount = 0;
    game->coinCount = 0;

    if (level->tiles) {
        free(level->tiles);
        level->tiles = NULL;
    }

    FILE *file = fopen(levelPath, "r");
    if (!file) {
        MessageBox(NULL, "Failed to load level", "Error", MB_OK);
        printf("Failed to load level: %s\n", levelPath);
        return 0;
    }

    fscanf(file, "%d %d %d %d %d", 
        &level->width, 
        &level->height, 
        &level->enemyAttackCooldown, 
        &level->playerDamage,
        &level->enemyBoxDropProbability
    );

    level->tiles = malloc(level->width * level->height * sizeof(int));
    level->enemyCount = 0;

    for (int row = 0; row < level->height; row++) {
        for (int col = 0; col < level->width; col++) {
            char c;
            fscanf(file, " %c", &c);

            int index = row * level->width + col;

            switch (c) {
                case '#':
                    level->tiles[index] = TILE_WALL;
                    break;

                case 'U':
                    level->tiles[index] = TILE_WALL_UP;
                    break;

                case 'D':
                    level->tiles[index] = TILE_WALL_DOWN;
                    break;

                case 'L':
                    level->tiles[index] = TILE_WALL_LEFT;
                    break;

                case 'R':
                    level->tiles[index] = TILE_WALL_RIGHT;
                    break;
                
                case 'T':
                    level->tiles[index] = TILE_WALL_TL;
                    break;

                case 'Y':
                    level->tiles[index] = TILE_WALL_TR;
                    break;

                case 'B':
                    level->tiles[index] = TILE_WALL_BL;
                    break;

                case 'N':
                    level->tiles[index] = TILE_WALL_BR;
                    break;

                case 'I':
                    level->tiles[index] = TILE_WALL_LU;
                    break;
                
                case 'O':
                    level->tiles[index] = TILE_WALL_RU;
                    break;
                
                case 'J':
                    level->tiles[index] = TILE_WALL_LD;
                    break;
                
                case 'K':
                    level->tiles[index] = TILE_WALL_RD;
                    break;

                case '.':
                    level->tiles[index] = TILE_FLOOR;
                    break;

                case 'P':
                    level->tiles[index] = TILE_FLOOR;
                    level->startX = col;
                    level->startY = row;
                    break;

                case 'E':
                    level->tiles[index] = TILE_FLOOR;

                    level->enemies[level->enemyCount].x = col * TILE_SIZE;
                    level->enemies[level->enemyCount].y = row * TILE_SIZE;
                    level->enemyCount++;
                    break;
                case 'G':
                    level->tiles[index] = TILE_GOAL_CLOSED;
                    level->goalIndex = index;
                    break;
            }
        }
    }

    fclose(file);

    Player_Init(game, level);
    Enemy_Init(game, level);
    Game_UpdateCamera(game);

    return 1;

}

int Spawn_Init(Game *game, int x, int y, SpawnType type) {
    Level *level = &game->levels[game->currentLevel];

    if (level->spawnCount >= MAX_SPAWNS) return 0;
    
    Spawn *spawn = &level->spawns[level->spawnCount++];
    spawn->x = x;
    spawn->y = y;
    spawn->type = type;
    spawn->anim = &game->spawnAnim[type];
    spawn->remove = 0;
}


void Game_Update(Game *game) {
    Player_Update(game);
    Enemy_Update(game);
    Game_UpdateCamera(game);

    if (Level_Won(game)) {
        game->levels[game->currentLevel].tiles[game->levels[game->currentLevel].goalIndex] = TILE_GOAL_OPEN;
    }
}


void Game_Render(Game *game, HWND hwnd) {
    if (game == NULL || hwnd == NULL) return;

    HDC hdc = GetDC(hwnd);

    HDC bufferDC = CreateCompatibleDC(hdc);
    HBITMAP bufferBitmap = CreateCompatibleBitmap(hdc, game->camera.width, game->camera.height);
    HBITMAP oldBitmap = SelectObject(bufferDC, bufferBitmap);

    RECT screenRect = {0, 0, game->camera.width, game->camera.height}; // x, y, width, height

    FillRect(bufferDC, &screenRect, (HBRUSH)(COLOR_WINDOW + 1)); // default background

    Level *level = &game->levels[game->currentLevel];

    int startCol = game->camera.x / TILE_SIZE;
    int endCol = (game->camera.x + game->camera.width) / TILE_SIZE;
    
    int startRow = game->camera.y / TILE_SIZE;
    int endRow = (game->camera.y + game->camera.height) / TILE_SIZE;

    if (startCol < 0) startCol = 0;
    if (startRow < 0) startRow = 0;
    if (endCol >= level->width) endCol = level->width - 1;
    if (endRow >= level->height) endRow = level->height - 1;

    HDC tileDC = CreateCompatibleDC(hdc);

    for (int row = startRow; row <= endRow; row++) {
        for (int col = startCol; col <= endCol; col++) {
            HBITMAP tileBitmap = NULL;
            int tile = level->tiles[row * level->width + col];

            int worldX = col * TILE_SIZE;
            int worldY = row * TILE_SIZE;

            int screenX = worldX - game->camera.x + game->camera.shakeOffsetX;
            int screenY = worldY - game->camera.y + game->camera.shakeOffsetY;

            if (tile == TILE_WALL) {
                tileBitmap = game->wallTile;
            } else if (tile == TILE_WALL_UP) {
                tileBitmap = game->wallUpTile;
            } else if (tile == TILE_WALL_DOWN) {
                tileBitmap = game->wallDownTile;
            } else if (tile == TILE_WALL_LEFT) {
                tileBitmap = game->wallLeftTile;
            } else if (tile == TILE_WALL_RIGHT) {
                tileBitmap = game->wallRightTile;
            } else if (tile == TILE_FLOOR) {
                tileBitmap = game->floorTile;
            } else if (tile == TILE_WALL_TL) {
                tileBitmap = game->wallTLTile;
            } else if (tile == TILE_WALL_TR) {
                tileBitmap = game->wallTRTile;
            } else if (tile == TILE_WALL_BL) {
                tileBitmap = game->wallBLTile;
            } else if (tile == TILE_WALL_BR) {
                tileBitmap = game->wallBRTile;
            } else if (tile == TILE_WALL_LU) {
                tileBitmap = game->wallLUTile;
            } else if (tile == TILE_WALL_RU) {
                tileBitmap = game->wallRUTile;
            } else if (tile == TILE_WALL_LD) {
                tileBitmap = game->wallLDTile;
            } else if (tile == TILE_WALL_RD) {
                tileBitmap = game->wallRDTile;
            } else if (tile == TILE_GOAL_CLOSED) {
                tileBitmap = game->goalClosedTile;
            } else if (tile == TILE_GOAL_OPEN) {
                tileBitmap = game->goalOpenTile;
            } else if (tile == TILE_OBS) {
                Ellipse(bufferDC, screenX, screenY, screenX + TILE_SIZE, screenY + TILE_SIZE);
            }

            if (tileBitmap) {
                SelectObject(tileDC, tileBitmap);
                TransparentBlt(bufferDC, screenX, screenY, TILE_SIZE, TILE_SIZE, tileDC, 0, 0, TILE_SIZE, TILE_SIZE, RGB(0, 0, 0));
            }
        }
    }
    DeleteDC(tileDC);

    if (game->gameOver) {
        if (Animation_Update(&game->gameOverAnim, 0)) {
            game->gameOverAnim.currentFrame = GAME_OVER_FRAMES - 1;
        }
        Game_Over(game, hdc, bufferDC);
    } else if (game->gameWin) {
        /*
        if (Animation_Update(&game->gameWinAnim, 0)) {
            game->gameWinAnim.currentFrame = GAME_WIN_FRAMES - 1;
        }
        Game_Win(game, hdc, bufferDC);
        */
        printf("You win!\n");
    }

    Player *player = &game->player;
    Player_Render(player, game, hdc, bufferDC);
    Enemy_Render(game, hdc, bufferDC);
    Spawn_Render(game, level->spawns, level->spawnCount, hdc, bufferDC);
    Coin_Render(game, game->coins, game->coinCount, hdc, bufferDC);

    BitBlt(hdc, 0, 0, game->camera.width, game->camera.height, bufferDC, 0, 0, SRCCOPY);
    SelectObject(bufferDC, oldBitmap);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDC);
    ReleaseDC(hwnd, hdc);
    
}

void Spawn_Render(Game *game, Spawn spawns[], int spawnCount, HDC hdc, HDC bufferDC) {
    for (int i = 0; i < spawnCount; i++) {
        Spawn *spawn = &spawns[i];
        if (spawn->remove) continue;

        Animation *anim = spawn->anim;
        
        int screenX = spawn->x - game->camera.x;
        int screenY = spawn->y - game->camera.y;

        HDC spriteDC = CreateCompatibleDC(hdc);
        SelectObject(spriteDC, anim->image);

        Animation_Update(anim, 0);
        int srcX = anim->currentFrame * anim->frameWidth;
        int srcY = 0;

        TransparentBlt(
            bufferDC,
            screenX,
            screenY,
            anim->frameWidth,
            anim->frameHeight,
            spriteDC,
            srcX,
            srcY,
            anim->frameWidth,
            anim->frameHeight,
            RGB(0, 0, 0)
        );
        DeleteDC(spriteDC);
    }
}

void Coin_Render(Game *game, Coin coins[], int coinCount, HDC hdc, HDC bufferDC) {
    for (int i = 0; i < coinCount; i++) {
        Coin *coin = &coins[i];
        if (coin->remove) continue;

        Animation *anim = &game->coinAnim;
        
        int screenX = coin->x - game->camera.x;
        int screenY = coin->y - game->camera.y;

        HDC spriteDC = CreateCompatibleDC(hdc);
        SelectObject(spriteDC, anim->image);

        Animation_Update(anim, 0);
        int srcX = anim->currentFrame * anim->frameWidth;
        int srcY = 0;

        TransparentBlt(
            bufferDC,
            screenX,
            screenY,
            anim->frameWidth,
            anim->frameHeight,
            spriteDC,
            srcX,
            srcY,
            anim->frameWidth,
            anim->frameHeight,
            RGB(0, 0, 0)
        );
        DeleteDC(spriteDC);

    }
}

void Game_Next_Level(Game *game) {
    game->currentLevel++;
    if (game->currentLevel >= game->levelCount) {
        game->gameWin = 1;
        return;
    }
    Game_InitLevel(game, levelPaths[game->currentLevel]);
}

static int Level_Won(Game *game) {
    Level *level = &game->levels[game->currentLevel];
    for (int i = 0; i < level->enemyCount; i++) {
        if (!level->enemies[i].remove) return 0;
    }
    return 1;
}

static void Game_Over(Game *game, HDC hdc, HDC bufferDC) {
    if (GetAsyncKeyState('R') & 0x0001) {
        game->gameOver = 0;
        game->gameOverAnim.currentFrame = 0;
        game->gameOverAnim.frameTimer = 0;

        Game_InitLevel(game, levelPaths[0]);
        return;
    }
    Animation *gameOverAnim = &game->gameOverAnim;
    HDC gameOverDC = CreateCompatibleDC(hdc);
    SelectObject(gameOverDC, gameOverAnim->image);

    int srcX = gameOverAnim->currentFrame * gameOverAnim->frameWidth;
    int srcY = 0;
    int x = game->camera.width / 2;
    x -= GAME_OVER_FRAME_WIDTH / 2;
    int y = game->camera.height / 2;
    y -= GAME_OVER_FRAME_HEIGHT / 2;

    TransparentBlt(
        bufferDC,
        x,
        y,
        GAME_OVER_FRAME_WIDTH,
        GAME_OVER_FRAME_HEIGHT,
        gameOverDC,
        srcX,
        srcY,
        GAME_OVER_FRAME_WIDTH,
        GAME_OVER_FRAME_HEIGHT,
        RGB(0, 0, 0)
    );
    DeleteDC(gameOverDC);
}

int RectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw &&
           ax + aw > bx &&
           ay < by + bh &&
           ay + ah > by;
}

int Animation_Update(Animation *animation, int direction) {
    animation->frameTimer++;

    if (animation->frameTimer >= animation->frameDelay) {
        animation->frameTimer = 0;
        animation->currentFrame++;

        if (animation->currentFrame >= animation->frameCount[direction]) {
            animation->currentFrame = 0;
            return 1;
        }
    }
    return 0;
}


static void Game_UpdateCamera(Game *game)
{
    Level *level = &game->levels[game->currentLevel];

    int levelWidth = level->width * TILE_SIZE;
    int levelHeight = level->height * TILE_SIZE;

    game->camera.x = game->player.x + game->player.spriteWidth / 2 - game->camera.width / 2;
    game->camera.y = game->player.y + game->player.spriteHeight / 2 - game->camera.height / 2;

    if (game->camera.x < 0) game->camera.x = 0;
    if (game->camera.y < 0) game->camera.y = 0;

    if (levelWidth > game->camera.width &&
        game->camera.x + game->camera.width > levelWidth)
        game->camera.x = levelWidth - game->camera.width;

    if (levelHeight > game->camera.height &&
        game->camera.y + game->camera.height > levelHeight)
        game->camera.y = levelHeight - game->camera.height;

    Camera_UpdateShake(&game->camera);
}

void Image_Init(Animation *anim, const char *path, int frameWidth, int frameHeight, int frameDelay, int *frameCounts) {
    Load_Image(&anim->image, path);

    anim->frameWidth = frameWidth;
    anim->frameHeight = frameHeight;
    anim->frameDelay = frameDelay;
    anim->currentFrame = 0;
    anim->frameTimer = 0;

    for (int i = 0; i < DIR_COUNT; i++) {
        anim->frameCount[i] = frameCounts[i];
    }
}

static void Load_Image(HBITMAP *bitmap, const char *path) {
    *bitmap = (HBITMAP)LoadImage(
        NULL,
        path,
        IMAGE_BITMAP,
        0,
        0,
        LR_LOADFROMFILE
    );

    if (!*bitmap) {
        char msg[256];
        wsprintf(msg, "Failed to load: %s\nError code: %lu", path, GetLastError());
        MessageBox(NULL, msg, "LoadImage Error", MB_OK);
    }
}


int Collision_Check(Game *game, int newX, int newY, int hitboxWidth, int hitboxHeight, int hitboxOffsetX, int hitboxOffsetY) {
    Level *level = &game->levels[game->currentLevel];
    
    int hitboxX = newX + hitboxOffsetX;
    int hitboxY = newY + hitboxOffsetY;

    int left = hitboxX / TILE_SIZE;
    int right = (hitboxX + hitboxWidth - 10) / TILE_SIZE;
    int top = hitboxY / TILE_SIZE;
    int bottom = (hitboxY + hitboxHeight - 10) / TILE_SIZE;

    if (left < 0 || right >= level->width || top < 0 || bottom >= level->height) {
        return 1;
    }

    if (level->tiles[top * level->width + left] != TILE_FLOOR) return 1;
    if (level->tiles[top * level->width + right] != TILE_FLOOR) return 1;
    if (level->tiles[bottom * level->width + left] != TILE_FLOOR) return 1;
    if (level->tiles[bottom * level->width + right] != TILE_FLOOR) return 1;

    return 0;
}

int Check_Distance_Range(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2, int range) {
    int centerX1 = x1 + width1 / 2;
    int centerY1 = y1 + height1 / 2;
    int centerX2 = x2 + width2 / 2;
    int centerY2 = y2 + height2 / 2;

    int dx = centerX1 - centerX2;
    int dy = centerY1 - centerY2;

    int inRange = (dx * dx + dy * dy) <= (range * range);
    return inRange;
}

SpawnType Random_Spawn() {
    switch (rand() % BOX_COUNT) {
        case 0: return HEALTH_BOX;
        case 1: return STRENGTH_BOX;
    }
}

void Apply_Spawn_Effect(Game *game, Spawn *spawn) {
    switch (spawn->type) {
        case HEALTH_BOX:
            int health = game->player.health + HEALTH_BOX_AMOUNT;
            game->player.health = (health < MAX_HEALTH) ? health : MAX_HEALTH;
            return;
        case STRENGTH_BOX:
            int strength = game->player.attackDamage + STRENGTH_BOX_AMOUNT;
            game->player.attackDamage = (strength < MAX_ATTACK_DAMAGE) ? strength : MAX_ATTACK_DAMAGE;
            return;
    }
}

int Create_Coin(Game *game, int x, int y, int value) {
    if (game->coinCount >= MAX_COINS) return 0;
    
    Coin *coin = &game->coins[game->coinCount++];
    coin->x = x;
    coin->y = y;
    coin->value = value;
    coin->remove = 0;
}

void Camera_Shake(Camera *camera, int duration, int strength) {
    camera->shakeTimer = duration;
    camera->shakeDuration = duration;
    camera->shakeStrength = strength;
}

void Camera_UpdateShake(Camera *camera) {
    camera->shakeOffsetX = 0;
    camera->shakeOffsetY = 0;

    if (camera->shakeTimer > 0) {
        camera->shakeOffsetX = (rand() % (camera->shakeStrength * 2 + 1)) - camera->shakeStrength;
        camera->shakeOffsetY = (rand() % (camera->shakeStrength * 2 + 1)) - camera->shakeStrength;

        camera->shakeTimer--;
    }
}