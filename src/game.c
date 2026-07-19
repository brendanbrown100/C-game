#include "game.h"
#include "main.h"
#include "player.h"
#include "enemy.h"
#include "handler.h"
#include "cannon.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>

static void Game_UpdateCamera(Game *game);
static int Level_Won(Game *game);
static void Jet_Update(Game *game);
static void Jet_Render(Game *game, HDC hdc, HDC bufferDC);
static void Game_Screen_Event(Game *game, Animation *anim, HDC hdc, HDC bufferDC);
static void Barrel_Update(Game *game);
static void Barrel_Render(Game *game, HDC hdc, HDC bufferDC);
static int Load_Key_Codes(Game *game);

const char *levelPaths[] = {
    LEVEL_1_PATH,
    LEVEL_2_PATH,
    LEVEL_3_PATH,
    LEVEL_4_PATH,
    LEVEL_5_PATH,
    LEVEL_6_PATH,
    LEVEL_7_PATH,
    LEVEL_8_PATH,
    LEVEL_9_PATH,
    LEVEL_10_PATH,
    LEVEL_11_PATH,
    LEVEL_12_PATH,
    LEVEL_13_PATH,
    LEVEL_14_PATH,
};


int Game_Init(Game *game) {
    if (game == NULL) return 0;
    srand((unsigned int) time(NULL));
    printf("%llu\n", (unsigned long long)sizeof(*game));
    

    game->levelCount = sizeof(levelPaths) / sizeof(levelPaths[0]);
    game->currentLevel = 0;
    game->gameOver = 0;
    game->gameWin = 0;
    game->backToMenu = 0;

    if (Load_Key_Codes(game)) printf("KeyCodes Loaded Successfully\n");
    
    Load_Image(&game->scoreImg, NUMBERS_PATH);

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
    Load_Image(&game->holeTile, HOLE_TILE_PATH);
    Load_Image(&game->arrowImg, ARROW_PATH);

    Load_Image(&game->jetAnim, JET_PATH);
    Load_Image(&game->bombAnim, BOMB_PATH);
    
    Load_Image(&game->barrelHorizAnim, BARREL_HORIZ_PATH);
    Load_Image(&game->barrelVertAnim, BARREL_VERT_PATH);

    



    Image_Init(
        &game->gameOverAnim,
        GAME_OVER_PATH,
        GAME_OVER_FRAME_WIDTH,
        GAME_OVER_FRAME_HEIGHT,
        GAME_OVER_FRAME_DELAY,
        (int[]){GAME_OVER_FRAMES, GAME_OVER_FRAMES, GAME_OVER_FRAMES, GAME_OVER_FRAMES}
    );

    Image_Init(
        &game->gameWinAnim,
        GAME_WIN_PATH,
        GAME_WIN_FRAME_WIDTH,
        GAME_WIN_FRAME_HEIGHT,
        GAME_WIN_FRAME_DELAY,
        (int[]){GAME_WIN_FRAMES, GAME_WIN_FRAMES, GAME_WIN_FRAMES, GAME_WIN_FRAMES}
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

    

    game->camera.width = WIDTH;
    game->camera.height = HEIGHT;
    game->camera.shakeTimer = 0;
    game->camera.shakeDuration = 0;
    game->camera.shakeStrength = 0;
    game->camera.shakeOffsetX = 0;
    game->camera.shakeOffsetY = 0;

    return 1;

}

int Game_InitLevel(Game *game, const char *levelPath) {
    Level *level = &game->levels[game->currentLevel];
    
    level->spawnCount = 0;
    game->coinCount = 0;
    game->carouselCount = 0;
    game->cannonCount = 0;
    game->arrowCount = 0;
    game->jetCount = 0;
    game->barrelCount = 0;

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

    if (fscanf(
        file,
        "%d %d %d %d %d %d",
        &level->width,
        &level->height,
        &level->enemyAttackCooldown,
        &level->playerDamage,
        &level->enemyBoxDropProbability,
        &level->enemySpeed
        ) != 6) {

        printf("Invalid level header: level %d\n", game->currentLevel + 1);
        fclose(file);
        return 0;
    }

    if (level->width <= 0 ||
        level->height <= 0 ||
        level->width > LEVEL_WIDTH ||
        level->height > LEVEL_HEIGHT) {

        printf(
            "Invalid level size: %d x %d\n",
            level->width,
            level->height
        );

        fclose(file);
        return 0;
    }

    level->tiles = malloc(
        (size_t)level->width *
        (size_t)level->height *
        sizeof *level->tiles
    );

    if (level->tiles == NULL) {
        printf("Failed to allocate level tiles\n");
        fclose(file);
        return 0;
    }
    level->enemyCount = 0;

    for (int row = 0; row < level->height; row++) {
        for (int col = 0; col < level->width; col++) {
            char c;
            fscanf(file, " %c", &c);

            int index = row * level->width + col;

            level->tiles[index] = TILE_FLOOR;

            switch (c) {
                case WALL:
                    level->tiles[index] = TILE_WALL;
                    break;

                case WALL_UP:
                    level->tiles[index] = TILE_WALL_UP;
                    break;

                case WALL_DOWN:
                    level->tiles[index] = TILE_WALL_DOWN;
                    break;

                case WALL_LEFT:
                    level->tiles[index] = TILE_WALL_LEFT;
                    break;

                case WALL_RIGHT:
                    level->tiles[index] = TILE_WALL_RIGHT;
                    break;
                
                case WALL_TL:
                    level->tiles[index] = TILE_WALL_TL;
                    break;

                case WALL_TR:
                    level->tiles[index] = TILE_WALL_TR;
                    break;

                case WALL_BL:
                    level->tiles[index] = TILE_WALL_BL;
                    break;

                case WALL_BR:
                    level->tiles[index] = TILE_WALL_BR;
                    break;

                case WALL_LU:
                    level->tiles[index] = TILE_WALL_LU;
                    break;
                
                case WALL_RU:
                    level->tiles[index] = TILE_WALL_RU;
                    break;
                
                case WALL_LD:
                    level->tiles[index] = TILE_WALL_LD;
                    break;
                
                case WALL_RD:
                    level->tiles[index] = TILE_WALL_RD;
                    break;

                case FLOOR:
                    level->tiles[index] = TILE_FLOOR;
                    break;
                
                case HOLE:
                    level->tiles[index] = TILE_HOLE;
                    break;

                case PLAYER_TILE:
                    level->startX = col;
                    level->startY = row;
                    break;

                case ENEMY_TILE:
                    if (level->enemyCount < MAX_ENEMIES) {
                        level->enemies[level->enemyCount].x = col * TILE_SIZE;
                        level->enemies[level->enemyCount].y = row * TILE_SIZE;
                        level->enemies[level->enemyCount].type = MELEE;
                        level->enemyCount++;
                    }
                    break;
                case ARCHER_TILE:
                    if (level->enemyCount < MAX_ENEMIES) {
                        level->enemies[level->enemyCount].x = col * TILE_SIZE;
                        level->enemies[level->enemyCount].y = row * TILE_SIZE;
                        level->enemies[level->enemyCount].type = ARCHER;
                        level->enemyCount++;
                    }
                    break;
                case GOAL_TILE:
                    level->tiles[index] = TILE_GOAL_CLOSED;
                    level->goalIndex = index;
                    break;
                case CAROUSEL_CC_TILE:
                    if (game->carouselCount < MAX_CAROUSELS) {
                        game->carousels[game->carouselCount].x = col * TILE_SIZE - (CAROUSEL_FRAME_WIDTH / 2) + (TILE_SIZE / 2);
                        game->carousels[game->carouselCount].y = row * TILE_SIZE - (CAROUSEL_FRAME_HEIGHT / 2) + (TILE_SIZE / 2);
                        game->carousels[game->carouselCount].clockWise = 0;
                        game->carouselCount++;
                    }
                    break;
                case CAROUSEL_CW_TILE:
                    if (game->carouselCount < MAX_CAROUSELS) {
                        game->carousels[game->carouselCount].x = col * TILE_SIZE - (CAROUSEL_FRAME_WIDTH / 2) + (TILE_SIZE / 2);
                        game->carousels[game->carouselCount].y = row * TILE_SIZE - (CAROUSEL_FRAME_HEIGHT / 2) + (TILE_SIZE / 2);
                        game->carousels[game->carouselCount].clockWise = 1;
                        game->carouselCount++;
                    }
                    break;
                case CANNON_UP:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_UP;
                        game->cannonCount++;
                    }
                    break;
                case CANNON_DOWN:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_DOWN;
                        game->cannonCount++;
                    }
                    break;
                case CANNON_LEFT:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_LEFT;
                        game->cannonCount++;
                    }
                    break;
                case CANNON_RIGHT:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_RIGHT;
                        game->cannonCount++;
                    }
                    break;
                case COIN_TILE:
                    Create_Coin(game, col * TILE_SIZE + 8, row * TILE_SIZE + 8, COIN_VALUE);
                    break;
                case COIN_HOLE_TILE:
                    Create_Coin(game, col * TILE_SIZE + 8, row * TILE_SIZE + 8, COIN_VALUE);
                    level->tiles[index] = TILE_HOLE;
                    break;
                case HEALTH_TILE:
                    Spawn_Init(game, col * TILE_SIZE, row * TILE_SIZE, HEALTH_BOX);
                    break;
                case JET_BOMB_TILE:
                    level->tiles[index] = TILE_BOMB;
                    break;
                case BARREL_TILE:
                    level->tiles[index] = TILE_FLOOR;
                    Spawn_Barrel(game, col * TILE_SIZE, row * TILE_SIZE);
                    break;
                
            }
        }
    }

    fclose(file);

    Player_Init(game, level);
    Enemy_Init(level);
    Carousel_Init(game);
    Cannon_Init(game);
    Game_UpdateCamera(game);

    return 1;

}

void Spawn_Barrel(Game *game, int x, int y) {
    Barrel *barrel = NULL;

    for (int i = 0; i < game->barrelCount; i++) {
        if (game->barrels[i].remove) {
            barrel = &game->barrels[i];
            break;
        }
    }

    if (barrel == NULL) {
        if (game->barrelCount >= MAX_BARRELS) {
            printf("FAILED TO INITILIZE BARREL: TOO MANY BARRELS\n");
            return;
        }

        barrel = &game->barrels[game->barrelCount];
        game->barrelCount++;
    }

    barrel->x = x;
    barrel->y = y;
    barrel->destroyed = 0;
    barrel->pickedUp = 0;
    barrel->frame = 0;
    barrel->speed = BARREL_SPEED;
    barrel->remove = 0;
    barrel->thrown = 0;
    barrel->dir = DIR_DOWN;
    barrel->isVerticle = 1;
    barrel->frameDelay = BARREL_FRAME_DELAY;
}

int Save_Game_Data(Game *game) {
    if (game == NULL) {
        return 0;
    }

    GameData data = {0};

    data.health = game->player.health;
    data.score = game->player.score;
    data.level = game->currentLevel;

    FILE *file = fopen(GAME_STATE_PATH, "wb");

    if (file == NULL) {
        printf("Failed to open save file\n");
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
        printf("Failed to write save data\n");
        return 0;
    }

    return 1;
}

int Game_Start_New(Game *game) {
    if (game == NULL) {
        return 0;
    }

    game->currentLevel = 0;
    game->gameOver = 0;
    game->gameWin = 0;
    game->backToMenu = 0;

    game->player.health = MAX_HEALTH;
    game->player.score = 0;

    if (!Game_InitLevel(game, levelPaths[0])) {
        printf("New game failed to load level 0\n");
        return 0;
    }

    return 1;
}

int Clear_Game_Data() {
    FILE *file = fopen(GAME_STATE_PATH, "wb");
    if (file == NULL) {
        printf("Failed to open save file - For clearing\n");
        return 0;
    }

    fclose(file);
    return 1;

}

static int Load_Key_Codes(Game *game) {
    FILE *file = fopen(GAME_KEY_CODES_PATH, "rb");

    if (file == NULL) {
        printf("FILE NOT FOUND: No key codes file found\n");
        game->upKeyCode = 87;
        game->downKeyCode = 83;
        game->leftKeyCode = 65;
        game->rightKeyCode = 68;
        game->sprintKeyCode = 16;
        game->dashKeyCode = 17;
        game->attackKeyCode = 32;
        game->interactKeyCode = 69;
        game->selectKeyCode = 13; 
        game->pauseKeyCode = 80;
        return 0;
    }

    KeyCodeData keyCodes = {0};

    size_t itemsRead = fread(
        &keyCodes,
        sizeof keyCodes,
        1,
        file
    );

    fclose(file);

    if (itemsRead != 1) {
        printf("FILE EMPTY: No key codes file found\n");
        game->upKeyCode = 87;
        game->downKeyCode = 83;
        game->leftKeyCode = 65;
        game->rightKeyCode = 68;
        game->sprintKeyCode = 16;
        game->dashKeyCode = 17;
        game->attackKeyCode = 32;
        game->interactKeyCode = 69;
        game->selectKeyCode = 13; 
        game->pauseKeyCode = 80;
        return 0;
    }

    if (keyCodes.upKeyCode <= 0 || keyCodes.upKeyCode >= 255) {
        printf("Invalid Up Key Code: %d\n", keyCodes.upKeyCode);
        return 0;
    } else if (keyCodes.downKeyCode <= 0 || keyCodes.downKeyCode >= 255) {
        printf("Invalid Down Key Code: %d\n", keyCodes.downKeyCode);
        return 0;
    } else if (keyCodes.leftKeyCode <= 0 || keyCodes.leftKeyCode >= 255) {
        printf("Invalid Left Key Code: %d\n", keyCodes.leftKeyCode);
        return 0;
    } else if (keyCodes.rightKeyCode <= 0 || keyCodes.rightKeyCode >= 255) {
        printf("Invalid Right Key Code: %d\n", keyCodes.rightKeyCode);
        return 0;
    } else if (keyCodes.sprintKeyCode <= 0 || keyCodes.sprintKeyCode >= 255) {
        printf("Invalid Sprint Key Code: %d\n", keyCodes.sprintKeyCode);
        return 0;
    } else if (keyCodes.dashKeyCode <= 0 || keyCodes.dashKeyCode >= 255) {
        printf("Invalid Dash Key Code: %d\n", keyCodes.dashKeyCode);
        return 0;
    } else if (keyCodes.attackKeyCode <= 0 || keyCodes.attackKeyCode >= 255) {
        printf("Invalid Attack Key Code: %d\n", keyCodes.attackKeyCode);
        return 0;
    } else if (keyCodes.interactKeyCode <= 0 || keyCodes.interactKeyCode >= 255) {
        printf("Invalid Interact Key Code: %d\n", keyCodes.interactKeyCode);
        return 0;
    } else if (keyCodes.selectKeyCode <= 0 || keyCodes.selectKeyCode >= 255) {
        printf("Invalid Select Key Code: %d\n", keyCodes.selectKeyCode);
        return 0;
    } 

    game->upKeyCode = keyCodes.upKeyCode;
    game->leftKeyCode = keyCodes.leftKeyCode;
    game->rightKeyCode = keyCodes.rightKeyCode;
    game->downKeyCode = keyCodes.downKeyCode;
    game->sprintKeyCode = keyCodes.sprintKeyCode;
    game->dashKeyCode = keyCodes.dashKeyCode;
    game->attackKeyCode = keyCodes.attackKeyCode;
    game->interactKeyCode = keyCodes.interactKeyCode;
    game->selectKeyCode = keyCodes.selectKeyCode;
    game->pauseKeyCode = keyCodes.pauseKeyCode;

    return 1;
}

int Load_Game_Data(Game *game) {
    if (game == NULL) {
        return 0;
    }

    FILE *file = fopen(GAME_STATE_PATH, "rb");

    if (file == NULL) {
        printf("No save file found\n");
        return 0;
    }

    GameData state = {0};

    size_t itemsRead = fread(
        &state,
        sizeof state,
        1,
        file
    );

    fclose(file);

    if (itemsRead != 1) {
        printf("Save file is empty or invalid\n");
        return 0;
    }

    /*
     * Validate against the actual number of available level paths,
     * not MAX_LEVELS.
     */
    if (state.level < 0 || state.level >= game->levelCount) {
        printf("Invalid saved level: %d\n", state.level);
        return 0;
    }

    if (state.health < 0 || state.health > MAX_HEALTH) {
        printf("Invalid saved health: %d\n", state.health);
        return 0;
    }

    game->currentLevel = state.level;
    game->gameOver = 0;
    game->gameWin = 0;
    game->backToMenu = 0;


    if (!Game_InitLevel(
            game,
            levelPaths[game->currentLevel])) {
        printf("Could not initialize saved level\n");
        return 0;
    }


    game->player.health = state.health;
    game->player.score = state.score;
    //game->player.attackDamage = state.attackDamage;



    return 1;
}

int Game_Has_Valid_Save(Game *game) {
    FILE *file = fopen(GAME_STATE_PATH, "rb");

    if (file == NULL) {
        return 0;
    }

    GameData state = {0};

    int valid =
        fread(&state, sizeof state, 1, file) == 1;

    fclose(file);

    if (!valid) {
        return 0;
    }

    if (state.level < 0 || state.level >= game->levelCount) {
        return 0;
    }

    if (state.health < 0 || state.health > MAX_HEALTH) {
        return 0;
    }

    return 1;
}

int Game_Restart_Current_Level(Game *game) {
    if (game == NULL) {
        return 0;
    }

    game->gameOver = 0;
    game->gameWin = 0;
    game->backToMenu = 0;

    game->player.attacking = 0;
    game->player.beenHit = 0;
    game->player.dead = 0;
    game->player.dashing = 0;

    if (!Game_InitLevel(game, levelPaths[game->currentLevel])) {
        printf("Failed to restart level %d\n",game->currentLevel);
        return 0;
    }

    return 1;
}


int Spawn_Init(Game *game, int x, int y, SpawnType type) {
    Level *level = &game->levels[game->currentLevel];

    if (level->spawnCount >= MAX_SPAWNS) return 0;
    
    Spawn *spawn = &level->spawns[level->spawnCount++];
    spawn->x = x;
    spawn->y = y;
    spawn->type = type;
    spawn->anim = game->spawnAnim[type];
    spawn->remove = 0;
    return 1;
}


void Game_Update(GameHandler *handler) {
    Player_Update(&handler->game);
    Barrel_Update(&handler->game);
    Enemy_Update(&handler->game);
    Carousel_Update(&handler->game);
    Cannon_Update(&handler->game);
    Game_UpdateCamera(&handler->game);
    Jet_Update(&handler->game);

    if (handler->game.backToMenu) {
        handler->game.backToMenu = 0;

        Home_Refresh_Continue(handler);

        handler->currState = MENU;
    }

    if (Level_Won(&handler->game)) {
        handler->game.levels[handler->game.currentLevel].tiles[handler->game.levels[handler->game.currentLevel].goalIndex] = TILE_GOAL_OPEN;
    }
}


void Game_Render(GameHandler *handler, HWND hwnd) {
    Game *game = &handler->game;
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
            } else if (tile == TILE_HOLE) {
                tileBitmap = game->holeTile;
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
            } else if (tile == TILE_BOMB) {
                tileBitmap = game->floorTile;
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
            game->backToMenu = 1;
        }
        Game_Screen_Event(game, &game->gameOverAnim, hdc, bufferDC);
    } else if (game->gameWin) {
        if (Animation_Update(&game->gameWinAnim, 0)) {
            game->gameWinAnim.currentFrame = GAME_WIN_FRAMES - 1;
        }
        Game_Screen_Event(game, &game->gameWinAnim, hdc, bufferDC);
    }

    Player *player = &game->player;
    Barrel_Render(game, hdc, bufferDC);
    Player_Render(player, game, hdc, bufferDC);
    Enemy_Render(game, hdc, bufferDC);
    Spawn_Render(game, level->spawns, level->spawnCount, hdc, bufferDC);
    Coin_Render(game, game->coins, game->coinCount, hdc, bufferDC);
    Carousel_Render(game, hdc, bufferDC);
    Cannon_Render(game, hdc, bufferDC);
    Jet_Render(game, hdc, bufferDC);

    Number_Render(game, SCORE_START_X, SCORE_START_Y, game->player.score, hdc, bufferDC);
    Number_Render(game, FPS_X, FPS_Y, (int)handler->fps, hdc, bufferDC);

    BitBlt(hdc, 0, 0, game->camera.width, game->camera.height, bufferDC, 0, 0, SRCCOPY);
    SelectObject(bufferDC, oldBitmap);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDC);
    ReleaseDC(hwnd, hdc);
    
}

static void Barrel_Update(Game *game) {
    Level *level = &game->levels[game->currentLevel];
    Player *player = &game->player;

    for (int i = 0; i < game->barrelCount; i++) {
        Barrel *barrel = &game->barrels[i];

        if (barrel->remove) {
            continue;
        }

        if (barrel->pickedUp) {
            continue;
        }

        if (barrel->destroyed) {
            barrel->frameDelay--;

            if (barrel->frameDelay <= 0) {
                barrel->frameDelay = BARREL_FRAME_DELAY;
                barrel->frame++;

                if (barrel->frame >= BARREL_FRAME_COUNT) {
                    barrel->remove = 1;
                }
            }

            continue;
        }

        if (!barrel->thrown) {
            continue;
        }

        int newX = barrel->x;
        int newY = barrel->y;

        switch (barrel->dir) {
            case DIR_DOWN:
                newY += barrel->speed;
                break;

            case DIR_UP:
                newY -= barrel->speed;
                break;

            case DIR_LEFT:
                newX -= barrel->speed;
                break;

            case DIR_RIGHT:
                newX += barrel->speed;
                break;
        }

        int hitboxOffsetX;
        int hitboxOffsetY;
        int hitboxWidth;
        int hitboxHeight;

        if (barrel->isVerticle) {
            hitboxOffsetX = BARREL_VERT_HITBOXOFFSET_X;
            hitboxOffsetY = BARREL_VERT_HITBOXOFFSET_Y;
            hitboxWidth = BARREL_VERT_WIDTH;
            hitboxHeight = BARREL_VERT_HEIGHT;
        } else {
            hitboxOffsetX = BARREL_HORIZ_HITBOXOFFSET_X;
            hitboxOffsetY = BARREL_HORIZ_HITBOXOFFSET_Y;
            hitboxWidth = BARREL_HORIZ_WIDTH;
            hitboxHeight = BARREL_HORIZ_HEIGHT;
        }

        if (Collision_Check(
                game,
                newX,
                newY,
                hitboxWidth,
                hitboxHeight,
                hitboxOffsetX,
                hitboxOffsetY)) {

            barrel->destroyed = 1;
            barrel->thrown = 0;
            barrel->frame = 1;
            barrel->frameDelay = BARREL_FRAME_DELAY;
            continue;
        }

        barrel->x = newX;
        barrel->y = newY;

        RECT barrelBox = {
            barrel->x + hitboxOffsetX,
            barrel->y + hitboxOffsetY,
            barrel->x + hitboxOffsetX + hitboxWidth,
            barrel->y + hitboxOffsetY + hitboxHeight
        };

        for (int j = 0; j < level->enemyCount; j++) {
            Enemy *enemy = &level->enemies[j];

            if (enemy->dead ||
                enemy->remove ||
                enemy->beenHit) {
                continue;
            }

            RECT enemyBox = {
                enemy->x + enemy->hitboxOffsetX,
                enemy->y + enemy->hitboxOffsetY,
                enemy->x + enemy->hitboxOffsetX + enemy->hitboxWidth,
                enemy->y + enemy->hitboxOffsetY + enemy->hitboxHeight
            };

            if (Rect_Overlap(barrelBox, enemyBox)) {
                barrel->destroyed = 1;
                barrel->thrown = 0;
                barrel->frame = 1;
                barrel->frameDelay = BARREL_FRAME_DELAY;

                enemy->beenHit = 1;
                enemy->state = ENEMY_HURT;
                enemy->attacking = 0;
                enemy->attackHit = 0;

                enemy->hurt.currentFrame = 0;
                enemy->hurt.frameTimer = 0;

                Enemy_Start_Knockback(game, enemy);

                int health =
                    enemy->health - player->attackDamage * 2;

                if (health <= 0) {
                    enemy->health = 0;
                    enemy->dead = 1;
                } else {
                    enemy->health = health;
                }

                break;
            }
        }
    }
}

void Number_Render(Game *game, int startX, int startY, int num, HDC hdc, HDC bufferDC) {
    int count = 0;
    int digits[20];

    if (num == 0) {
        HDC scoreDC = CreateCompatibleDC(hdc);
        SelectObject(scoreDC, game->scoreImg);
        TransparentBlt(
            bufferDC,
            startX,
            startY,
            NUMBERS_FRAME_WIDTH,
            NUMBERS_FRAME_HEIGHT,
            scoreDC,
            0,
            0,
            NUMBERS_FRAME_WIDTH,
            NUMBERS_FRAME_HEIGHT,
            RGB(0, 0, 0)
        );
        DeleteDC(scoreDC);
        return;
    }

    while (num > 0) {
        digits[count] = num % 10;
        count++;
        num /= 10;
    }

    for (int i = count - 1; i >= 0; i--) {
        int j = (count - 1) - i;
        int x = (NUMBERS_FRAME_WIDTH * j) + startX;
        int y = startY;
        HDC scoreDC = CreateCompatibleDC(hdc);
        SelectObject(scoreDC, game->scoreImg);
        int srcX = digits[i] * NUMBERS_FRAME_WIDTH;
        TransparentBlt(
            bufferDC,
            x,
            y,
            NUMBERS_FRAME_WIDTH,
            NUMBERS_FRAME_HEIGHT,
            scoreDC,
            srcX,
            0,
            NUMBERS_FRAME_WIDTH,
            NUMBERS_FRAME_HEIGHT,
            RGB(0, 0, 0)
        );
        DeleteDC(scoreDC);
    }
}



void Spawn_Render(Game *game, Spawn spawns[], int spawnCount, HDC hdc, HDC bufferDC) {
    for (int i = 0; i < spawnCount; i++) {
        Spawn *spawn = &spawns[i];
        if (spawn->remove) continue;

        Animation *anim = &spawn->anim;
        
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

static void Barrel_Render(Game *game, HDC hdc, HDC bufferDC) {
    HDC barrelDC = CreateCompatibleDC(hdc);

    for (int i = 0; i < game->barrelCount; i++) {
        Barrel *barrel = &game->barrels[i];

        if (barrel->remove) {
            continue;
        }

        /*
         * The player carrying animation already shows the barrel.
         * Do not draw the world barrel while carried.
         */
        if (barrel->pickedUp) {
            continue;
        }

        HBITMAP img =
            barrel->isVerticle ?
            game->barrelVertAnim :
            game->barrelHorizAnim;

        HBITMAP oldImage =
            SelectObject(barrelDC, img);

        int screenX = barrel->x - game->camera.x;
        int screenY = barrel->y - game->camera.y;

        int srcX = barrel->frame * BARREL_WIDTH;
        int srcY = 0;

        TransparentBlt(
            bufferDC,
            screenX,
            screenY,
            BARREL_WIDTH,
            BARREL_HEIGHT,
            barrelDC,
            srcX,
            srcY,
            BARREL_WIDTH,
            BARREL_HEIGHT,
            RGB(0, 0, 0)
        );

        SelectObject(barrelDC, oldImage);
    }

    DeleteDC(barrelDC);
}

void Game_Next_Level(Game *game) {
    int next = game->currentLevel + 1;
    if (next >= game->levelCount) {
        game->gameWin = 1;
        return;
    }
    game->currentLevel = next;
    if (!Game_InitLevel(game, levelPaths[game->currentLevel])) {
        printf("Failed to load level %d", game->currentLevel);
        return;
    }
    Save_Game_Data(game);
}

static int Level_Won(Game *game) {
    Level *level = &game->levels[game->currentLevel];
    for (int i = 0; i < level->enemyCount; i++) {
        if (!level->enemies[i].remove) return 0;
    }
    return 1;
}

void Spawn_Jet(Game *game) {
    Jet *jet = NULL;

    for (int i = 0; i < game->jetCount; i++) {
        if (game->jets[i].remove) {
            jet = &game->jets[i];
            break;
        }
    }

    if (jet == NULL) {
        if (game->jetCount >= MAX_JETS) {
            printf("FAILED TO INITILIZE JET: TOO MANY JETS\n");
            return;
        }

        jet = &game->jets[game->jetCount];
        game->jetCount++;
    }
    jet->x = game->camera.x + game->camera.width + JET_FRAME_WIDTH;
    jet->y = game->player.y;
    jet->speed = JET_SPEED;
    jet->remove = 0;

    New_Image_Init(&jet->anim, JET_FRAME_WIDTH, JET_FRAME_HEIGHT, JET_FRAME_DELAY, (int[]){JET_FRAMES, JET_FRAMES, JET_FRAMES, JET_FRAMES});
    Bomb *bomb = &jet->bomb;
    bomb->x = 0;
    bomb->y = 0;
    bomb->explode = 0;
    bomb->radius = 0;
    bomb->damage = BOMB_DAMAGE;
    bomb->remove = 1;
    bomb->dealtDamage = 0;
    New_Image_Init(&bomb->anim, BOMB_FRAME_WIDTH, BOMB_FRAME_HEIGHT, BOMB_FRAME_DELAY, (int[]){BOMB_FRAMES, BOMB_FRAMES, BOMB_FRAMES, BOMB_FRAMES});

}

static void Jet_Update(Game *game)
{
    for (int i = 0; i < game->jetCount; i++) {
        Jet *jet = &game->jets[i];
        Bomb *bomb = &jet->bomb;

        if (!jet->remove) {
            New_Animation_Update(&jet->anim, 0);

            jet->x -= jet->speed;

            if (jet->x < game->player.x && bomb->remove) {
                bomb->x = jet->x + (JET_FRAME_WIDTH / 2)
                                  - (BOMB_FRAME_WIDTH / 2);

                bomb->y = jet->y + (JET_FRAME_HEIGHT / 2) 
                                    - (BOMB_FRAME_HEIGHT / 2);

                bomb->remove = 0;
                bomb->explode = 0;
                bomb->radius = 0;
                bomb->dealtDamage = 0;
                bomb->anim.currentFrame = 0;
                bomb->anim.frameTimer = 0;
            }

            if (jet->x + JET_FRAME_WIDTH < game->camera.x) {
                jet->remove = 1;
            }
        }

        if (bomb->remove) {
            continue;
        }

        int finished = New_Animation_Update(&bomb->anim, 0);

        if (bomb->anim.currentFrame >= BOMB_EXPLODE_FRAME) {
            int frame =
                bomb->anim.currentFrame - BOMB_EXPLODE_FRAME;

            int radius =
                EXPL_START_SIZE + EXPL_SIZE_INCREMENT * frame;

            int bombOffset =
                (BOMB_FRAME_WIDTH - radius) / 2;

            int explosionX = bomb->x + bombOffset;
            int explosionY = bomb->y + bombOffset;

            int playerX =
                game->player.x + game->player.hitboxOffsetX;

            int playerY =
                game->player.y + game->player.hitboxOffsetY;

            if (!bomb->dealtDamage &&
                RectsOverlap(
                    explosionX,
                    explosionY,
                    radius,
                    radius,
                    playerX,
                    playerY,
                    game->player.hitboxWidth,
                    game->player.hitboxHeight
                )) {

                game->player.health -= bomb->damage;

                if (game->player.health <= 0) {
                    game->player.health = 0;
                    game->player.dead = 1;
                } else {
                    game->player.beenHit = 1;
                }

                bomb->dealtDamage = 1;

                Camera_Shake(
                    &game->camera,
                    PLAYER_HIT_SHAKE_DURATION,
                    PLAYER_HIT_SHAKE_STRENGTH
                );
            }
        }
        if (finished) {
            bomb->remove = 1;
            continue;
        }
    }
}

static void Jet_Render(Game *game, HDC hdc, HDC bufferDC) {
    for (int i = 0; i < game->jetCount; i++) {
        Jet *jet = &game->jets[i];
        if (jet->remove) goto bomb_render;

        int jetX = jet->x - game->camera.x;
        int jetY = jet->y - game->camera.y;

        if (jetX < -JET_FRAME_WIDTH || 
            jetX > game->camera.width + JET_FRAME_WIDTH || 
            jetY < -JET_FRAME_HEIGHT || 
            jetY > game->camera.height + JET_FRAME_HEIGHT) continue;
        
        HDC jetDC = CreateCompatibleDC(hdc);
        SelectObject(jetDC, game->jetAnim);
        int srcX = jet->anim.currentFrame * jet->anim.frameWidth;
        int srcY = 0;

        TransparentBlt(
            bufferDC,
            jetX,
            jetY,
            jet->anim.frameWidth,
            jet->anim.frameHeight,
            jetDC,
            srcX,
            srcY,
            jet->anim.frameWidth,
            jet->anim.frameHeight,
            RGB(0, 0, 0)
        );
        DeleteDC(jetDC);

        bomb_render:
        Bomb *bomb = &jet->bomb;

        if (bomb->remove) continue;

        int bombX = bomb->x - game->camera.x;
        int bombY = bomb->y - game->camera.y;

        if (bombX < -BOMB_FRAME_WIDTH || 
            bombX > game->camera.width + BOMB_FRAME_WIDTH || 
            bombY < -BOMB_FRAME_HEIGHT || 
            bombY > game->camera.height + BOMB_FRAME_HEIGHT) continue;
        
        HDC bombDC = CreateCompatibleDC(hdc);
        SelectObject(bombDC, game->bombAnim);

        int srX = bomb->anim.currentFrame * bomb->anim.frameWidth;
        int srY = 0;

        TransparentBlt(
            bufferDC,
            bombX,
            bombY,
            bomb->anim.frameWidth,
            bomb->anim.frameHeight,
            bombDC,
            srX,
            srY,
            bomb->anim.frameWidth,
            bomb->anim.frameHeight,
            RGB(0, 0, 0)
        );
        DeleteDC(bombDC);
        
    }
}

static void Game_Screen_Event(Game *game, Animation *anim, HDC hdc, HDC bufferDC) {
    HDC gameEventDC = CreateCompatibleDC(hdc);
    SelectObject(gameEventDC, anim->image);

    int srcX = anim->currentFrame * anim->frameWidth;
    int srcY = 0;
    int x = game->camera.width / 2;
    x -= anim->frameWidth / 2;
    int y = game->camera.height / 2;
    y -= anim->frameHeight / 2;

    TransparentBlt(
        bufferDC,
        x,
        y,
        anim->frameWidth,
        anim->frameHeight,
        gameEventDC,
        srcX,
        srcY,
        anim->frameWidth,
        anim->frameHeight,
        RGB(0, 0, 0)
    );
    DeleteDC(gameEventDC);
}


int RectsOverlap(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw &&
           ax + aw > bx &&
           ay < by + bh &&
           ay + ah > by;
}

int New_Animation_Update(NewAnimation *animation, int direction) {
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

void New_Image_Init(NewAnimation *anim, int frameWidth, int frameHeight, int frameDelay, int *frameCounts) {
    anim->frameWidth = frameWidth;
    anim->frameHeight = frameHeight;
    anim->frameDelay = frameDelay;
    anim->currentFrame = 0;
    anim->frameTimer = 0;

    for (int i = 0; i < DIR_COUNT; i++) {
        anim->frameCount[i] = frameCounts[i];
    }
}

void Load_Image(HBITMAP *bitmap, const char *path) {
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

    int left = (hitboxX + 8) / TILE_SIZE;
    int right = (hitboxX + hitboxWidth - 9) / TILE_SIZE;
    int top = (hitboxY + 8) / TILE_SIZE;
    int bottom = (hitboxY + hitboxHeight - 9) / TILE_SIZE;

    if (left < 0 || right >= level->width || top < 0 || bottom >= level->height) {
        return 1;
    }

    int tLeftTile = level->tiles[top * level->width + left];
    int tRightTile = level->tiles[top * level->width + right];
    int bLeftTile = level->tiles[bottom * level->width + left];
    int bRightTile = level->tiles[bottom * level->width + right];

    if (tLeftTile != TILE_FLOOR && tLeftTile != TILE_HOLE && tLeftTile != TILE_BOMB) return 1;
    if (tRightTile != TILE_FLOOR && tRightTile != TILE_HOLE && tRightTile != TILE_BOMB) return 1;
    if (bLeftTile != TILE_FLOOR && bLeftTile != TILE_HOLE && bLeftTile != TILE_BOMB) return 1;
    if (bRightTile != TILE_FLOOR && bRightTile != TILE_HOLE && bRightTile != TILE_BOMB) return 1;

    return 0;
}

int Check_Fall(Game *game, int newX, int newY, int hitboxWidth, int hitboxHeight, int hitboxOffsetX, int hitboxOffsetY) {
    Level *level = &game->levels[game->currentLevel];

    int hitboxX = newX + hitboxOffsetX;
    int hitboxY = newY + hitboxOffsetY;

    int feetHeight = 6;

    int feetLeft = (hitboxX + 10) / TILE_SIZE;
    int feetRight = (hitboxX + hitboxWidth - 11) / TILE_SIZE;
    int feetY = (hitboxY + hitboxHeight - feetHeight) / TILE_SIZE;

    if (feetLeft < 0 ||
        feetRight >= level->width ||
        feetY < 0 ||
        feetY >= level->height) {

        return 0;
    }

    int leftTile = level->tiles[feetY * level->width + feetLeft];
    int rightTile = level->tiles[feetY * level->width + feetRight];

    int ret = leftTile == TILE_HOLE && rightTile == TILE_HOLE;
    if (leftTile == TILE_BOMB && rightTile == TILE_BOMB) {
        level->tiles[feetY * level->width + feetLeft] = TILE_FLOOR;
        return 2;
    } else if (ret) return 1;
    else return 0;
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

void Get_Attack_Box(int x, int y, int hitboxOffsetX, int hitboxOffsetY, int hitboxWidth, int hitboxHeight, int dir, int range, RECT *attackBox) {
    int bodyX = x + hitboxOffsetX;
    int bodyY = y + hitboxOffsetY;

    attackBox->left = bodyX;
    attackBox->top = bodyY;
    attackBox->right = bodyX + hitboxWidth;
    attackBox->bottom = bodyY + hitboxHeight;

    if (dir == DIR_LEFT) {
        attackBox->left -= range;
        attackBox->right -= hitboxWidth;
    } else if (dir == DIR_RIGHT) {
        attackBox->left += hitboxWidth;
        attackBox->right += range;
    } else if (dir == DIR_UP) {
        attackBox->top -= range;
        attackBox->bottom -= hitboxHeight;
    } else if (dir == DIR_DOWN) {
        attackBox->top += hitboxHeight;
        attackBox->bottom += range;
    }
}

int Rect_Overlap(RECT a, RECT b) {
    return a.left < b.right &&
           a.right > b.left &&
           a.top < b.bottom &&
           a.bottom > b.top;
}

SpawnType Random_Spawn() {
    switch (rand() % BOX_COUNT) {
        case 0: return HEALTH_BOX;
        case 1: return STRENGTH_BOX;
        default: return HEALTH_BOX;
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
        case BOX_COUNT:
            return;
    }
}

int Create_Coin(Game *game, int x, int y, int value) {
    if (game->coinCount >= MAX_COINS) {
        printf("FAILED TO INITILIZE COIN: TOO MANY COINS\n");
        return 0;
    }
    
    Coin *coin = &game->coins[game->coinCount++];
    coin->x = x;
    coin->y = y;
    coin->value = value;
    coin->remove = 0;
    return 1;
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