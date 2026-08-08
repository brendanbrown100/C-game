#include "game.h"
#include "main.h"
#include "player.h"
#include "enemy.h"
#include "handler.h"
#include "cannon.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

static void Game_UpdateCamera(Game *game);
static int Level_Won(Game *game);
static void Jet_Update(Game *Game);
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
    

    game->levelCount = sizeof(levelPaths) / sizeof(levelPaths[0]);
    game->currentLevel = 0;
    game->gameOver = 0;
    game->gameWin = 0;
    game->backToMenu = 0;
    game->numPlayers = 1;

    if (!Load_Key_Codes(game)) {
        printf("ERROR: Load_Key_Codes failed\n");
        return 0;
    }
    

    if (!Load_Image(&game->wallTile, WALL_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallUpTile, WALL_UP_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallDownTile, WALL_DOWN_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallLeftTile, WALL_LEFT_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallRightTile, WALL_RIGHT_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallTLTile, WALL_TL_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallTRTile, WALL_TR_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallBLTile, WALL_BL_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallBRTile, WALL_BR_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallLUTile, WALL_LU_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallRUTile, WALL_RU_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallLDTile, WALL_LD_TILE_PATH)) return 0;
    if (!Load_Image(&game->wallRDTile, WALL_RD_TILE_PATH)) return 0;
    if (!Load_Image(&game->goalClosedTile, GOAL_CLOSED_TILE_PATH)) return 0;
    if (!Load_Image(&game->goalOpenTile, GOAL_OPEN_TILE_PATH)) return 0;
    if (!Load_Image(&game->floorTile, FLOOR_TILE_PATH)) return 0;
    if (!Load_Image(&game->holeTile, HOLE_TILE_PATH)) return 0;
    if (!Load_Image(&game->arrowImg, ARROW_PATH)) return 0;

    if (!Load_Image(&game->jetAnim, JET_PATH)) return 0;
    if (!Load_Image(&game->bombAnim, BOMB_PATH)) return 0;
    
    if (!Load_Image(&game->barrelHorizAnim, BARREL_HORIZ_PATH)) return 0;
    if (!Load_Image(&game->barrelVertAnim, BARREL_VERT_PATH)) return 0;

    



    if (!Image_Init(
        &game->gameOverAnim,
        GAME_OVER_PATH,
        GAME_OVER_FRAME_WIDTH,
        GAME_OVER_FRAME_HEIGHT,
        GAME_OVER_FRAME_DELAY,
        (int[]){GAME_OVER_FRAMES, GAME_OVER_FRAMES, GAME_OVER_FRAMES, GAME_OVER_FRAMES}
    )) return 0;

    if (!Image_Init(
        &game->gameWinAnim,
        GAME_WIN_PATH,
        GAME_WIN_FRAME_WIDTH,
        GAME_WIN_FRAME_HEIGHT,
        GAME_WIN_FRAME_DELAY,
        (int[]){GAME_WIN_FRAMES, GAME_WIN_FRAMES, GAME_WIN_FRAMES, GAME_WIN_FRAMES}
    )) return 0;

    if (!Image_Init(
        &game->spawnAnim[HEALTH_BOX],
        HEALTH_BOX_PATH,
        SPAWN_BOX_FRAME_WIDTH,
        SPAWN_BOX_FRAME_HEIGHT,
        SPAWN_BOX_FRAME_DELAY,
        (int[]){SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES}
    )) return 0;

    if (!Image_Init(
        &game->spawnAnim[STRENGTH_BOX],
        STRENGTH_BOX_PATH,
        SPAWN_BOX_FRAME_WIDTH,
        SPAWN_BOX_FRAME_HEIGHT,
        SPAWN_BOX_FRAME_DELAY,
        (int[]){SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES, SPAWN_BOX_FRAMES}
    )) return 0;

    if (!Image_Init(
        &game->coinAnim,
        COIN_PATH,
        COIN_FRAME_WIDTH,
        COIN_FRAME_HEIGHT,
        COIN_FRAME_DELAY,
        (int[]){COIN_FRAMES, COIN_FRAMES, COIN_FRAMES, COIN_FRAMES}
    )) return 0;

    

    game->camera.width = WIDTH;
    game->camera.height = HEIGHT;
    game->camera.damping = CAMERA_DAMPING;
    game->camera.shakeTimer = 0;
    game->camera.shakeStrength = 0;
    game->camera.shakeOffsetX = 0;
    game->camera.shakeOffsetY = 0;

    return 1;

}

int Game_InitLevel(Game *game, const char *levelPath) {
    Level *level = &game->level;
    
    level->spawnCount = 0;
    game->coinCount = 0;
    game->carouselCount = 0;
    game->cannonCount = 0;
    game->arrowCount = 0;
    game->jetCount = 0;
    game->barrelCount = 0;

    game->time = 0.0;

    if (level->tiles) {
        free(level->tiles);
        level->tiles = NULL;
    }

    FILE *file = fopen(levelPath, "r");
    if (!file) {
        MessageBox(NULL, "Failed to load level", "Error", MB_OK);
        printf("ERROR: FILE NOT FOUND - Failed to load level: %s\n", levelPath);
        return 0;
    }

    if (fscanf(
        file,
        "width:%d height:%d enemyCoolDown:%f playerDamage:%d enemySpawnProb:%d enemySpeed:%d",
        &level->width,
        &level->height,
        &level->enemyAttackCooldown,
        &level->playerDamage,
        &level->enemyBoxDropProbability,
        &level->enemySpeed
        ) != 6) {

        printf("ERROR: FILE FORMAT - Invalid level header: level %d\n", game->currentLevel + 1);
        fclose(file);
        return 0;
    }

    if (level->width <= 0 ||
        level->height <= 0 ||
        level->width > LEVEL_WIDTH ||
        level->height > LEVEL_HEIGHT) {

        printf("ERROR: Invalid level size: %d x %d\n", level->width, level->height);
        fclose(file);
        return 0;
    }

    size_t tileCount =
        (size_t)level->width *
        (size_t)level->height;

    size_t allocationSize =
        tileCount * sizeof(*level->tiles);

    level->tiles = malloc(allocationSize);

    if (level->tiles == NULL) {
        printf("ERROR: MALLOC FAIL - Failed to allocate level tiles\n");
        fclose(file);
        return 0;
    }

    printf("Level %d size: %llu bytes\n", game->currentLevel + 1, (unsigned long long)allocationSize);

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
                        level->enemies[level->enemyCount].x = col * TILE_SIZE - 16;
                        level->enemies[level->enemyCount].y = row * TILE_SIZE - 16;
                        level->enemies[level->enemyCount].type = MELEE;
                        level->enemies[level->enemyCount].hitboxWidth = ENEMY_WIDTH;
                        level->enemies[level->enemyCount].hitboxHeight = ENEMY_HEIGHT;
                        level->enemies[level->enemyCount].hitboxOffsetX = (ENEMY_FRAME_WIDTH - ENEMY_WIDTH) / 2;
                        level->enemies[level->enemyCount].hitboxOffsetY = (ENEMY_FRAME_HEIGHT - ENEMY_HEIGHT) / 2;
                        level->enemies[level->enemyCount].speed = 0;
                        level->enemies[level->enemyCount].health = 0;
                        level->enemies[level->enemyCount].hasSpawn = 0;
                        level->enemies[level->enemyCount].damage = 0;
                        level->enemyCount++;
                    } else {
                        printf("ERROR: failed to initilize enemy - too many enemies\n");
                        return 0;
                    }
                    break;
                case ARCHER_TILE:
                    if (level->enemyCount < MAX_ENEMIES) {
                        level->enemies[level->enemyCount].x = col * TILE_SIZE - 16;
                        level->enemies[level->enemyCount].y = row * TILE_SIZE - 16;
                        level->enemies[level->enemyCount].type = ARCHER;
                        level->enemies[level->enemyCount].hitboxWidth = ARCHER_WIDTH;
                        level->enemies[level->enemyCount].hitboxHeight = ARCHER_HEIGHT;
                        level->enemies[level->enemyCount].hitboxOffsetX = (ENEMY_FRAME_WIDTH - ARCHER_WIDTH) / 2;
                        level->enemies[level->enemyCount].hitboxOffsetY = (ENEMY_FRAME_HEIGHT - ARCHER_HEIGHT) / 2;
                        level->enemies[level->enemyCount].speed = 0;
                        level->enemies[level->enemyCount].health = 0;
                        level->enemies[level->enemyCount].hasSpawn = 0;
                        level->enemies[level->enemyCount].damage = 0;
                        level->enemyCount++;
                    } else {
                        printf("ERROR: failed to initilize enemy - too many enemies\n");
                        return 0;
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
                        game->carousels[game->carouselCount].damage = 0;
                        game->carousels[game->carouselCount].frameDelay = 0;
                        game->carouselCount++;
                    } else {
                        printf("ERROR: failed to initilize carousel - too many carousels\n");
                        return 0;
                    }
                    break;
                case CAROUSEL_CW_TILE:
                    if (game->carouselCount < MAX_CAROUSELS) {
                        game->carousels[game->carouselCount].x = col * TILE_SIZE - (CAROUSEL_FRAME_WIDTH / 2) + (TILE_SIZE / 2);
                        game->carousels[game->carouselCount].y = row * TILE_SIZE - (CAROUSEL_FRAME_HEIGHT / 2) + (TILE_SIZE / 2);
                        game->carousels[game->carouselCount].clockWise = 1;
                        game->carousels[game->carouselCount].damage = 0;
                        game->carousels[game->carouselCount].frameDelay = 0;
                        game->carouselCount++;
                    } else {
                        printf("ERROR: failed to initilize carousel - too many carousels\n");
                        return 0;
                    }
                    break;
                case CANNON_UP:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_UP;
                        game->cannons[game->cannonCount].attackDelay = 0;
                        game->cannons[game->cannonCount].bulletSpeed = 0;
                        game->cannons[game->cannonCount].damage = 0;
                        game->cannonCount++;
                    } else {
                        printf("ERROR: failed to initilize cannons - too many cannons\n");
                        return 0;
                    }
                    break;
                case CANNON_DOWN:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_DOWN;
                        game->cannons[game->cannonCount].attackDelay = 0;
                        game->cannons[game->cannonCount].bulletSpeed = 0;
                        game->cannons[game->cannonCount].damage = 0;
                        game->cannonCount++;
                    } else {
                        printf("ERROR: failed to initilize cannon - too many cannons\n");
                        return 0;
                    }
                    break;
                case CANNON_LEFT:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_LEFT;
                        game->cannons[game->cannonCount].attackDelay = 0;
                        game->cannons[game->cannonCount].bulletSpeed = 0;
                        game->cannons[game->cannonCount].damage = 0;
                        game->cannonCount++;
                    } else {
                        printf("ERROR: failed to initilize cannon - too many cannons\n");
                        return 0;
                    }
                    break;
                case CANNON_RIGHT:
                    if (game->cannonCount < MAX_CANNONS) {
                        game->cannons[game->cannonCount].x = col * TILE_SIZE;
                        game->cannons[game->cannonCount].y = row * TILE_SIZE;
                        game->cannons[game->cannonCount].direction = DIR_RIGHT;
                        game->cannons[game->cannonCount].attackDelay = 0;
                        game->cannons[game->cannonCount].bulletSpeed = 0;
                        game->cannons[game->cannonCount].damage = 0;
                        game->cannonCount++;
                    } else {
                        printf("ERROR: failed to initilize cannon - too many cannons\n");
                        return 0;
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

    char name[31];
    while (fscanf(file, " name:%30s ", name) == 1) {
        int index;
        if (strcmp(name, "enemy") == 0) {
            int health, damage, speed, hasSpawn;
            if (fscanf(file, "idx:%d health:%d damage:%d speed:%d hasSpawn:%d", &index, &health, &damage, &speed, &hasSpawn) != 5) {
                printf("ERROR: MAP - %s has invalid format\n", name);
                return 0;
            } else if (index < 0 || index >= game->level.enemyCount ||
                health <= 0 ||
                damage <= 0 ||
                speed <= 0 ||
                hasSpawn < 0 || hasSpawn > 1) {
                    printf("ERROR: MAP - %s has invalid atributes\n", name);
                    return 0;
            }
            Enemy *enemy = &game->level.enemies[index];
            enemy->health = health;
            enemy->damage = damage;
            enemy->speed = speed;
            enemy->hasSpawn = hasSpawn;
        } else if (strcmp(name, "cannon") == 0) {
            int damage, bulletSpeed;
            float attackSpeed;
            if (fscanf(file, "idx:%d damage:%d bulletSpeed:%d attackSpeed:%f", &index, &damage, &bulletSpeed, &attackSpeed) != 4) {
                printf("ERROR: MAP - %s has invalid format\n", name);
                return 0;
            }
            if (index < 0 || index >= game->cannonCount ||
                damage <= 0 ||
                bulletSpeed <= 0 ||
                attackSpeed <= 0) {
                    printf("ERROR: MAP - %s has invalid atributes\n", name);
                    return 0;
            }
            Cannon *cannon = &game->cannons[index];
            cannon->damage = damage;
            cannon->bulletSpeed = bulletSpeed;
            cannon->attackDelay = attackSpeed;
        } else if (strcmp(name, "carousel") == 0) {
            int damage;
            float frameDelay;
            if (fscanf(file, "idx:%d damage:%d frameDelay:%f", &index, &damage, &frameDelay) != 3) {
                printf("ERROR: MAP - %s has invalid format\n", name);
                return 0;
            }
            if (index < 0 || index >= game->carouselCount ||
                damage <= 0 ||
                frameDelay <= 0) {
                    printf("ERROR: MAP - %s has invalid atributes\n", name);
                    return 0;
                }
            Carousel *carousel = &game->carousels[index];
            carousel->damage = damage;
            carousel->frameDelay = frameDelay;
        }
    } 

    fclose(file);
    for (int i = 0; i < game->numPlayers; i++) {
        Player_Init(game, level, i);
    }
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
            printf("ERROR: failed to initilize barrel - too many barrels\n");
            return;
        }

        barrel = &game->barrels[game->barrelCount];
        game->barrelCount++;
    }

    barrel->x = (float)x;
    barrel->y = (float)y;
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

    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerData *playerData = &data.playerData[i];
        playerData->health = game->players[i].health;
    }
    data.score = game->score;
    data.level = game->currentLevel;
    data.numPlayers = game->numPlayers;
    data.damping = game->camera.damping;


    FILE *file = fopen(GAME_STATE_PATH, "wb");

    if (file == NULL) {
        printf("WARNING: failed to open save file\n");
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
        printf("WARNING: failed to write save data\n");
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
    game->score = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player *player = &game->players[i];
        player->health = MAX_HEALTH;
        player->dead = 0;
    }

    if (!Game_InitLevel(game, levelPaths[0])) {
        printf("ERROR: failed to load new game\n");
        return 0;
    }

    return 1;
}

int Clear_Game_Data() {
    FILE *file = fopen(GAME_STATE_PATH, "wb");
    if (file == NULL) {
        printf("WARNING: FILE NOT FOUND - save file for clearing\n");
        return 0;
    }

    fclose(file);
    return 1;

}

static int Load_Key_Codes(Game *game) {
    FILE *file = fopen(GAME_KEY_CODES_PATH, "rb");

    if (file == NULL) {
        printf("WARNING: FILE NOT FOUND - No key codes file found\n");
        PlayerKeyCodeData *p1 = &game->playerKeyCodeData[0];
        PlayerKeyCodeData *p2 = &game->playerKeyCodeData[1];
        PlayerKeyCodeData *p3 = &game->playerKeyCodeData[2];
        PlayerKeyCodeData *p4 = &game->playerKeyCodeData[3];

        p1->upKeyCode = 87; // 'W'
        p1->downKeyCode = 83; // 'S'
        p1->leftKeyCode = 65; // 'A'
        p1->rightKeyCode = 68; // 'D'
        p1->sprintKeyCode = 16; // SHIFT
        p1->dashKeyCode = 17; // CNTRL
        p1->attackKeyCode = 32; // SPACE
        p1->interactKeyCode = 69; // 'E'
        p1->selectKeyCode = 13; // ENTER
        p1->pauseKeyCode = 80; // 'P'

        p2->upKeyCode = 38; // UP key
        p2->downKeyCode = 40; // DOWN key
        p2->leftKeyCode = 37; // LEFT key
        p2->rightKeyCode = 39; // RIGHT key
        p2->sprintKeyCode = 18; // ALT
        p2->dashKeyCode = 77; // 'M' key
        p2->attackKeyCode = 78; // 'N' key
        p2->interactKeyCode = 75; // 'K' key
        p2->selectKeyCode = 76; // 'L' key
        p2->pauseKeyCode = 74; // 'J' key

        p3->upKeyCode = 0;
        p3->downKeyCode = 0;
        p3->leftKeyCode = 0;
        p3->rightKeyCode = 0;
        p3->sprintKeyCode = 0; 
        p3->dashKeyCode = 0; 
        p3->attackKeyCode = 0; 
        p3->interactKeyCode = 0; 
        p3->selectKeyCode = 0; 
        p3->pauseKeyCode = 0; 

        p4->upKeyCode = 0;
        p4->downKeyCode = 0;
        p4->leftKeyCode = 0;
        p4->rightKeyCode = 0;
        p4->sprintKeyCode = 0; 
        p4->dashKeyCode = 0; 
        p4->attackKeyCode = 0; 
        p4->interactKeyCode = 0; 
        p4->selectKeyCode = 0; 
        p4->pauseKeyCode = 0; 
        return 1;
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
        printf("WARNING: FILE FORMAT - key codes file has incorrect format\n");
        PlayerKeyCodeData *p1 = &game->playerKeyCodeData[0];
        PlayerKeyCodeData *p2 = &game->playerKeyCodeData[1];
        PlayerKeyCodeData *p3 = &game->playerKeyCodeData[2];
        PlayerKeyCodeData *p4 = &game->playerKeyCodeData[3];

        p1->upKeyCode = 87; // 'W'
        p1->downKeyCode = 83; // 'S'
        p1->leftKeyCode = 65; // 'A'
        p1->rightKeyCode = 68; // 'D'
        p1->sprintKeyCode = 16; // SHIFT
        p1->dashKeyCode = 17; // CNTRL
        p1->attackKeyCode = 32; // SPACE
        p1->interactKeyCode = 69; // 'E'
        p1->selectKeyCode = 13; // ENTER
        p1->pauseKeyCode = 80; // 'P'

        p2->upKeyCode = 38; // UP key
        p2->downKeyCode = 40; // DOWN key
        p2->leftKeyCode = 37; // LEFT key
        p2->rightKeyCode = 39; // RIGHT key
        p2->sprintKeyCode = 18; // ALT
        p2->dashKeyCode = 77; // 'M' key
        p2->attackKeyCode = 78; // 'N' key
        p2->interactKeyCode = 75; // 'K' key
        p2->selectKeyCode = 76; // 'L' key
        p2->pauseKeyCode = 74; // 'J' key

        p3->upKeyCode = 0;
        p3->downKeyCode = 0;
        p3->leftKeyCode = 0;
        p3->rightKeyCode = 0;
        p3->sprintKeyCode = 0; 
        p3->dashKeyCode = 0; 
        p3->attackKeyCode = 0; 
        p3->interactKeyCode = 0; 
        p3->selectKeyCode = 0; 
        p3->pauseKeyCode = 0; 

        p4->upKeyCode = 0;
        p4->downKeyCode = 0;
        p4->leftKeyCode = 0;
        p4->rightKeyCode = 0;
        p4->sprintKeyCode = 0; 
        p4->dashKeyCode = 0; 
        p4->attackKeyCode = 0; 
        p4->interactKeyCode = 0; 
        p4->selectKeyCode = 0; 
        p4->pauseKeyCode = 0; 

        return 1;
    }

    int playerKeyCodeError = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerKeyCodeData *playerkeyCodes = &keyCodes.playerKeyCodeData[i];
        if (playerkeyCodes->upKeyCode < 0 || playerkeyCodes->upKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Up Key Code: %d\n", i + 1, playerkeyCodes->upKeyCode);
            playerKeyCodeError = 1;
        } 
        if (playerkeyCodes->downKeyCode < 0 || playerkeyCodes->downKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Down Key Code: %d\n", i + 1, playerkeyCodes->downKeyCode);
            playerKeyCodeError = 1;
        } 
        if (playerkeyCodes->leftKeyCode < 0 || playerkeyCodes->leftKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Left Key Code: %d\n", i + 1, playerkeyCodes->leftKeyCode);
        } 
        if (playerkeyCodes->rightKeyCode < 0 || playerkeyCodes->rightKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Right Key Code: %d\n", i + 1, playerkeyCodes->rightKeyCode);
            playerKeyCodeError = 1;
        } 
        if (playerkeyCodes->sprintKeyCode < 0 || playerkeyCodes->sprintKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Sprint Key Code: %d\n", i + 1, playerkeyCodes->sprintKeyCode);
            playerKeyCodeError = 1;
        } 
        if (playerkeyCodes->dashKeyCode < 0 || playerkeyCodes->dashKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Dash Key Code: %d\n", i + 1, playerkeyCodes->dashKeyCode);
            playerKeyCodeError = 1;
        } 
        if (playerkeyCodes->attackKeyCode < 0 || playerkeyCodes->attackKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Attack Key Code: %d\n", i + 1, playerkeyCodes->attackKeyCode);
            playerKeyCodeError = 1;
        } 
        if (playerkeyCodes->interactKeyCode < 0 || playerkeyCodes->interactKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Interact Key Code: %d\n", i + 1, playerkeyCodes->interactKeyCode);
            playerKeyCodeError = 1;
        } 
        if (playerkeyCodes->selectKeyCode < 0 || playerkeyCodes->selectKeyCode >= 255) {
            printf("WARNING - PLAYER %d - Invalid Select Key Code: %d\n", i + 1, playerkeyCodes->selectKeyCode);
            playerKeyCodeError = 1;
        } 

        if (!playerKeyCodeError) {
            game->playerKeyCodeData[i].upKeyCode = playerkeyCodes->upKeyCode;
            game->playerKeyCodeData[i].leftKeyCode = playerkeyCodes->leftKeyCode;
            game->playerKeyCodeData[i].rightKeyCode = playerkeyCodes->rightKeyCode;
            game->playerKeyCodeData[i].downKeyCode = playerkeyCodes->downKeyCode;
            game->playerKeyCodeData[i].sprintKeyCode = playerkeyCodes->sprintKeyCode;
            game->playerKeyCodeData[i].dashKeyCode = playerkeyCodes->dashKeyCode;
            game->playerKeyCodeData[i].attackKeyCode = playerkeyCodes->attackKeyCode;
            game->playerKeyCodeData[i].interactKeyCode = playerkeyCodes->interactKeyCode;
            game->playerKeyCodeData[i].selectKeyCode = playerkeyCodes->selectKeyCode;
            game->playerKeyCodeData[i].pauseKeyCode = playerkeyCodes->pauseKeyCode;
        }
    }
    if (playerKeyCodeError) return 0;
    return 1;
}

int Load_Game_Data(Game *game) {
    if (game == NULL) {
        return 0;
    }

    FILE *file = fopen(GAME_STATE_PATH, "rb");

    if (file == NULL) {
        printf("WARNING: FILE NOT FOUND - no game data file found\n");
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
        printf("WARNING: FILE FORMAT - game data file is empty or invalid\n");
        return 0;
    }

    /*
     * Validate against the actual number of available level paths,
     * not MAX_LEVELS.
     */
    if (state.level < 0 || state.level >= game->levelCount) {
        printf("WARNING: Invalid saved level: %d\n", state.level);
        return 0;
    }
    if (state.damping <= 0 || state.damping > 1) {
        printf("WARNING: Invalid saved damping: %.2f\n", state.damping);
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerData *playerData = &state.playerData[i];
        if (playerData->health < 0 || playerData->health > MAX_HEALTH) {
            printf("WARNING: Invalid saved health player %d: %d\n", i + 1, playerData->health);
            return 0;
        
        }
    }

    game->currentLevel = state.level;
    game->score = state.score;
    game->gameOver = 0;
    game->gameWin = 0;
    game->backToMenu = 0;


    if (!Game_InitLevel(game, levelPaths[game->currentLevel])) {
        printf("ERROR: could not initialize saved level\n");
        return -1;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        game->players[i].health = state.playerData[i].health;     
    }
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

    for (int i = 0; i < MAX_PLAYERS; i++) {
        PlayerData *playerData = &state.playerData[i];
        if (playerData->health < 0 || playerData->health > MAX_HEALTH) {
            return 0;
        }
    }

    return 1;
}

int Game_Restart_Current_Level(Game *game) {
    game->gameOver = 0;
    game->gameWin = 0;
    game->backToMenu = 0;

    for (int i = 0; i < game->numPlayers; i++) {
        Player *player = &game->players[i];
        player->attacking = 0;
        player->beenHit = 0;
        player->dead = 0;
        player->dashing = 0;
    }

    if (!Game_InitLevel(game, levelPaths[game->currentLevel])) {
        printf("ERROR: Failed to restart level %d\n",game->currentLevel);
        return 0;
    }
    return 1;
}


int Spawn_Init(Game *game, int x, int y, SpawnType type) {
    Level *level = &game->level;

    if (level->spawnCount >= MAX_SPAWNS) {
        printf("WARNING: failed to initilize spawn - too many spawns\n");
        return 0;
    }
    
    Spawn *spawn = &level->spawns[level->spawnCount++];
    spawn->x = x;
    spawn->y = y;
    spawn->type = type;
    spawn->anim = game->spawnAnim[type];
    spawn->remove = 0;
    return 1;
}


int Game_Update(GameHandler *handler) {
    int numAlive = 0;
    for (int i = 0; i < handler->game.numPlayers; i++) {
        if (handler->game.players[i].remove) numAlive++;
        if (!Player_Update(handler, i)) {
            printf("ERROR: Player_Update failed\n");
            return 0;
        }
    }
    handler->game.gameOver = numAlive == handler->game.numPlayers;
    Barrel_Update(&handler->game);
    if (!Enemy_Update(&handler->game)) {
        printf("Error: Enemy_Update failed\n");
        return 0;
    }
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
        handler->game.level.tiles[handler->game.level.goalIndex] = TILE_GOAL_OPEN;
    }
    return 1;
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

    Level *level = &game->level;

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
        if (Animation_Update(&game->gameOverAnim, 0, game->deltaTime)) {
            game->gameOverAnim.currentFrame = 0;
            game->backToMenu = 1;
        }
        Game_Screen_Event(game, &game->gameOverAnim, hdc, bufferDC);
    } else if (game->gameWin) {
        if (Animation_Update(&game->gameWinAnim, 0, game->deltaTime)) {
            game->gameWinAnim.currentFrame = 0;
        }
        Game_Screen_Event(game, &game->gameWinAnim, hdc, bufferDC);
    }

    Coin_Render(game, game->coins, game->coinCount, hdc, bufferDC);
    Spawn_Render(game, level->spawns, level->spawnCount, hdc, bufferDC);
    Barrel_Render(game, hdc, bufferDC);
    for (int i = 0; i < game->numPlayers; i++) {
        Player_Render(handler, i, hdc, bufferDC);
    }
    Carousel_Render(game, hdc, bufferDC);
    Cannon_Render(game, hdc, bufferDC);
    Jet_Render(game, hdc, bufferDC);
    Enemy_Render(game, hdc, bufferDC);

    Number_Render(handler, SCORE_START_X, SCORE_START_Y, game->score, STRING_ORANGE, hdc, bufferDC);
    Number_Render(handler, FPS_X, FPS_Y, (int)handler->fps, STRING_BLUE, hdc, bufferDC);
    Number_Render(handler, TIME_X, TIME_Y, (int)game->time, STRING_BLUE, hdc, bufferDC);
    BitBlt(hdc, 0, 0, game->camera.width, game->camera.height, bufferDC, 0, 0, SRCCOPY);
    SelectObject(bufferDC, oldBitmap);
    DeleteObject(bufferBitmap);
    DeleteDC(bufferDC);
    ReleaseDC(hwnd, hdc);
    
}


static void Barrel_Update(Game *game) {
    Level *level = &game->level;

    for (int i = 0; i < game->barrelCount; i++) {
        Barrel *barrel = &game->barrels[i];

        if (barrel->remove) {
            continue;
        }

        if (barrel->pickedUp) {
            continue;
        }

        if (barrel->destroyed) {
            barrel->frameDelay -= game->deltaTime;

            if (barrel->frameDelay <= 0.0f) {
                barrel->frameDelay = BARREL_FRAME_DELAY;
                barrel->frame++;

                if (barrel->frame >= BARREL_FRAME_COUNT) {
                    barrel->remove = 1;
                }
            }
        }

        if (!barrel->thrown) {
            continue;
        }

        float newX = barrel->x;
        float newY = barrel->y;

        switch (barrel->dir) {
            case DIR_DOWN:
                newY += barrel->speed * game->deltaTime;
                break;

            case DIR_UP:
                newY -= barrel->speed * game->deltaTime;
                break;

            case DIR_LEFT:
                newX -= barrel->speed * game->deltaTime;
                break;

            case DIR_RIGHT:
                newX += barrel->speed * game->deltaTime;
                break;
        }

        int hitboxOffsetX;
        int hitboxOffsetY;
        int hitboxWidth;
        int hitboxHeight;

        if (barrel->isVerticle) {
            hitboxOffsetX = BARREL_VERT_HITBOXOFFSET_X;
            hitboxOffsetY = 11;
            hitboxWidth = BARREL_VERT_WIDTH;
            hitboxHeight = 10;
        } else {
            hitboxOffsetX = 11;
            hitboxOffsetY = BARREL_HORIZ_HITBOXOFFSET_Y;
            hitboxWidth = 10;
            hitboxHeight = BARREL_HORIZ_HEIGHT;
        }

        if (Collision_Check(
                game,
                (int)newX,
                (int)newY,
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

        if (barrel->isVerticle) {
            hitboxOffsetY = BARREL_VERT_HITBOXOFFSET_Y;
            hitboxHeight = BARREL_VERT_HEIGHT;
        } else {
            hitboxOffsetX = BARREL_HORIZ_HITBOXOFFSET_X;
            hitboxWidth = BARREL_HORIZ_WIDTH;
        }

        barrel->x = newX;
        barrel->y = newY;

        RECT barrelBox = {
            (int)barrel->x + hitboxOffsetX,
            (int)barrel->y + hitboxOffsetY,
            (int)barrel->x + hitboxOffsetX + hitboxWidth,
            (int)barrel->y + hitboxOffsetY + hitboxHeight
        };

        for (int j = 0; j < level->enemyCount; j++) {
            Enemy *enemy = &level->enemies[j];

            if (enemy->dead ||
                enemy->remove ||
                enemy->beenHit) {
                continue;
            }

            RECT enemyBox = {
                (int)enemy->x + enemy->hitboxOffsetX,
                (int)enemy->y + enemy->hitboxOffsetY,
                (int)enemy->x + enemy->hitboxOffsetX + enemy->hitboxWidth,
                (int)enemy->y + enemy->hitboxOffsetY + enemy->hitboxHeight
            };

            if (Rect_Overlap(barrelBox, enemyBox)) {
                barrel->destroyed = 1;
                barrel->thrown = 0;
                barrel->frame = 1;
                barrel->frameDelay = BARREL_FRAME_DELAY;

                barrelBox.top -= BARREL_BREAK_HITBOX_INCREASE_Y;
                barrelBox.left -= BARREL_BREAK_HITBOX_INCREASE_X;
                barrelBox.bottom += BARREL_BREAK_HITBOX_INCREASE_Y;
                barrelBox.right += BARREL_BREAK_HITBOX_INCREASE_X;
                

                enemy->beenHit = 1;
                enemy->state = ENEMY_HURT;
                enemy->attacking = 0;
                enemy->attackHit = 0;

                enemy->hurt.currentFrame = 0;
                enemy->hurt.frameTimer = 0.0f;

                Enemy_Start_Knockback(enemy, (int)barrel->x, (int)barrel->y);

                int health = enemy->health - game->level.playerDamage * 2;

                if (health <= 0) {
                    enemy->health = 0;
                    enemy->dead = 1;
                } else {
                    enemy->health = health;
                }
            }
        }
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
        Animation_Update(anim, 0, game->deltaTime);

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

        Animation_Update(anim, 0, game->deltaTime);
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
        // if (barrel->pickedUp) {
        //     continue;
        // }

        HBITMAP img =
            barrel->isVerticle ?
            game->barrelVertAnim :
            game->barrelHorizAnim;

        HBITMAP oldImage =
            SelectObject(barrelDC, img);

        int screenX = (int)barrel->x - game->camera.x;
        int screenY = (int)barrel->y - game->camera.y;

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

int Game_Next_Level(Game *game) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player *player = &game->players[i];
        if (player->health <= 0) player->health = 20;
    }

    int next = game->currentLevel + 1;
    if (next >= game->levelCount) {
        game->gameWin = 1;
        return 1;
    }
    game->currentLevel = next;
    if (!Game_InitLevel(game, levelPaths[game->currentLevel])) {
        printf("ERROR: Failed to load level %d\n", game->currentLevel);
        return 0;
    }
    if (!Save_Game_Data(game)) {
        printf("ERROR: Failed to save game data\n");
        return 0;
    }
    return 1;
}

static int Level_Won(Game *game) {
    Level *level = &game->level;
    for (int i = 0; i < level->enemyCount; i++) {
        if (!level->enemies[i].remove) return 0;
    }
    return 1;
}

void Spawn_Jet(Game *game, int pIndex) {
    Jet *jet = NULL;

    for (int i = 0; i < game->jetCount; i++) {
        if (game->jets[i].remove) {
            jet = &game->jets[i];
            break;
        }
    }

    if (jet == NULL) {
        if (game->jetCount >= MAX_JETS) {
            printf("WARNING: failed to initilize jet - too many jets\n");
            return;
        }

        jet = &game->jets[game->jetCount];
        game->jetCount++;
    }
    jet->x = game->camera.x + game->camera.width + JET_FRAME_WIDTH;
    jet->y = game->players[pIndex].y;
    jet->speed = JET_SPEED;
    jet->remove = 0;
    jet->player = pIndex;

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

static void Jet_Update(Game *game) {
    for (int i = 0; i < game->jetCount; i++) {
        Jet *jet = &game->jets[i];
        Bomb *bomb = &jet->bomb;

        if (!jet->remove) {
            New_Animation_Update(&jet->anim, 0, game->deltaTime);

            jet->x -= jet->speed * game->deltaTime;

            if (jet->x < (int)game->players[jet->player].x && bomb->remove) {
                bomb->x = jet->x + (JET_FRAME_WIDTH / 2)
                                  - (BOMB_FRAME_WIDTH / 2);

                bomb->y = jet->y + (JET_FRAME_HEIGHT / 2) 
                                    - (BOMB_FRAME_HEIGHT / 2);

                bomb->remove = 0;
                bomb->explode = 0;
                bomb->radius = 0;
                bomb->dealtDamage = 0;
                bomb->anim.currentFrame = 0;
                bomb->anim.frameTimer = 0.0f;
            }

            if (jet->x + JET_FRAME_WIDTH < game->camera.x) {
                jet->remove = 1;
            }
        }

        if (bomb->remove) {
            continue;
        }

        int finished = New_Animation_Update(&bomb->anim, 0, game->deltaTime);

        if (bomb->anim.currentFrame >= BOMB_EXPLODE_FRAME) {
            int frame =
                bomb->anim.currentFrame - BOMB_EXPLODE_FRAME;

            int radius =
                EXPL_START_SIZE + EXPL_SIZE_INCREMENT * frame;

            int bombOffset =
                (BOMB_FRAME_WIDTH - radius) / 2;

            int explosionX = bomb->x + bombOffset;
            int explosionY = bomb->y + bombOffset;
            for (int i = 0; i < game->numPlayers; i++) {
                Player *player = &game->players[i];

                int playerX = (int)player->x + player->hitboxOffsetX;
                int playerY = (int)player->y + player->hitboxOffsetY;

                if (!bomb->dealtDamage &&
                    RectsOverlap(
                        explosionX,
                        explosionY,
                        radius,
                        radius,
                        playerX,
                        playerY,
                        player->hitboxWidth,
                        player->hitboxHeight
                    )) {

                    Player_Hit(game, i, bomb->damage);
                    bomb->dealtDamage = 1;
                }
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

int New_Animation_Update(NewAnimation *animation, int direction, float deltaTime) {
    animation->frameTimer += deltaTime;

    if (animation->frameTimer >= animation->frameDelay) {
        animation->frameTimer = 0.0f;
        animation->currentFrame++;

        if (animation->currentFrame >= animation->frameCount[direction]) {
            animation->currentFrame = 0;
            return 1;
        }
    }
    return 0;
}

int Animation_Update(Animation *animation, int direction, float deltaTime) {
    animation->frameTimer += deltaTime;

    if (animation->frameTimer >= animation->frameDelay) {
        animation->frameTimer = 0.0f;
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
    Level *level = &game->level;

    int levelWidth = level->width * TILE_SIZE;
    int levelHeight = level->height * TILE_SIZE;
    
    int camX = 0;
    int camY = 0;
    int numPlayerAlive = 0;
    for (int i = 0; i < game->numPlayers; i++) {
        Player *player = &game->players[i];
        if (player->remove) continue;
        numPlayerAlive++;
        camX += (int)player->x + player->spriteWidth / 2 - game->camera.width / 2;
        camY += (int)player->y + player->spriteHeight / 2 - game->camera.height / 2;
    }
    if (numPlayerAlive == 0) return;

    float alpha = 1.0f - powf(game->camera.damping, game->deltaTime * 30.0f);
    
    game->camera.exactX += ((camX / numPlayerAlive) - game->camera.x) * alpha;
    game->camera.exactY += ((camY / numPlayerAlive) - game->camera.y) * alpha;

    if (game->camera.exactX < 0) game->camera.exactX = 0;
    if (game->camera.exactY < 0) game->camera.exactY = 0;

    if (levelWidth > game->camera.width &&
        game->camera.exactX + game->camera.width > levelWidth)
        game->camera.exactX = levelWidth - game->camera.width;

    if (levelHeight > game->camera.height &&
        game->camera.exactY + game->camera.height > levelHeight)
        game->camera.exactY = levelHeight - game->camera.height;
    
    game->camera.x = (int)game->camera.exactX;
    game->camera.y = (int)game->camera.exactY;

    Camera_UpdateShake(&game->camera, game->deltaTime);
}

int Image_Init(Animation *anim, const char *path, int frameWidth, int frameHeight, float frameDelay, int *frameCounts) {
    if (!Load_Image(&anim->image, path)) return 0;

    anim->frameWidth = frameWidth;
    anim->frameHeight = frameHeight;
    anim->frameDelay = frameDelay;
    anim->currentFrame = 0;
    anim->frameTimer = 0.0f;

    for (int i = 0; i < DIR_COUNT; i++) {
        anim->frameCount[i] = frameCounts[i];
    }
    return 1;
}

void New_Image_Init(NewAnimation *anim, int frameWidth, int frameHeight, float frameDelay, int *frameCounts) {
    anim->frameWidth = frameWidth;
    anim->frameHeight = frameHeight;
    anim->frameDelay = frameDelay;
    anim->currentFrame = 0;
    anim->frameTimer = 0.0f;

    for (int i = 0; i < DIR_COUNT; i++) {
        anim->frameCount[i] = frameCounts[i];
    }
}

int Load_Image(HBITMAP *bitmap, const char *path) {
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
        return 0;
    }
    return 1;
}


int Collision_Check(Game *game, int newX, int newY, int hitboxWidth, int hitboxHeight, int hitboxOffsetX, int hitboxOffsetY) {
    Level *level = &game->level;
    
    int hitboxX = newX + hitboxOffsetX;
    int hitboxY = newY + hitboxOffsetY;

    int left = (hitboxX) / TILE_SIZE;
    int right = (hitboxX + hitboxWidth) / TILE_SIZE;
    int top = (hitboxY) / TILE_SIZE;
    int bottom = (hitboxY + hitboxHeight) / TILE_SIZE;

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
    Level *level = &game->level;

    int hitboxX = newX + hitboxOffsetX;
    int hitboxY = newY + hitboxOffsetY;

    int feetHeight = 3;

    int feetLeft = (hitboxX + 5) / TILE_SIZE;
    int feetRight = (hitboxX + hitboxWidth - 5) / TILE_SIZE;
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

    int distance = (dx * dx + dy * dy);
    int inRange = distance <= (range * range);
    return inRange ? distance : 0;
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

void Apply_Spawn_Effect(Game *game, Spawn *spawn, int pIndex) {
    switch (spawn->type) {
        case HEALTH_BOX:
            int health = game->players[pIndex].health + HEALTH_BOX_AMOUNT;
            game->players[pIndex].health = (health < MAX_HEALTH) ? health : MAX_HEALTH;
            return;
        case STRENGTH_BOX:
            int strength = game->players[pIndex].attackDamage + STRENGTH_BOX_AMOUNT;
            game->players[pIndex].attackDamage = (strength < MAX_ATTACK_DAMAGE) ? strength : MAX_ATTACK_DAMAGE;
            return;
        case BOX_COUNT:
            return;
    }
}

int Create_Coin(Game *game, int x, int y, int value) {
    if (game->coinCount >= MAX_COINS) {
        printf("WARNING: failed to initilize coin: too many coins\n");
        return 0;
    }
    
    Coin *coin = &game->coins[game->coinCount++];
    coin->x = x;
    coin->y = y;
    coin->value = value;
    coin->remove = 0;
    return 1;
}

void Camera_Shake(Camera *camera, float duration, int strength) {
    camera->shakeTimer = duration;
    camera->shakeStrength = strength;
}

void Camera_UpdateShake(Camera *camera, float deltaTime) {
    camera->shakeOffsetX = 0;
    camera->shakeOffsetY = 0;

    if (camera->shakeTimer > 0) {
        camera->shakeOffsetX = (rand() % (camera->shakeStrength * 2 + 1)) - camera->shakeStrength;
        camera->shakeOffsetY = (rand() % (camera->shakeStrength * 2 + 1)) - camera->shakeStrength;

        camera->shakeTimer -= deltaTime;
    }
}