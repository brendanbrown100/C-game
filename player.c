#include "player.h"
#include "game.h"
#include "main.h"
#include <windows.h>
#include <stdio.h>

static void HealthBar_Update(Player *player);
static void Player_CheckAttackHit(Game *game);
static Animation *Player_GetCurrentAnimation(Player *player);
static void Check_Player_Interact(Game *game);
static void Check_Player_Spawn(Game *game);

void Player_Init(Game *game, Level *level) {
    Player *player = &game->player;

    player->x = level->startX * TILE_SIZE;
    player->y = level->startY * TILE_SIZE;
    player->hitboxWidth = PLAYER_WIDTH;
    player->hitboxHeight = PLAYER_HEIGHT;
    player->hitboxOffsetX = (FRAME_WIDTH - PLAYER_WIDTH) / 2;
    player->hitboxOffsetY = (FRAME_HEIGHT - PLAYER_HEIGHT) / 2;;
    player->spriteWidth = FRAME_WIDTH;
    player->spriteHeight = FRAME_HEIGHT;
    player->health = MAX_HEALTH;
    player->score = 0;
    player->sprinting = 0;
    player->moving = 0;
    player->direction = DIR_DOWN;
    player->state = PLAYER_IDLE;
    player->attacking = 0;
    player->beenHit = 0;
    player->dead = 0;
    player->attackDirection = DIR_DOWN;
    player->attackMoveSpeed = 0;
    player->attackDamage = level->playerDamage;

    Image_Init(
        &player->healthBar, 
        HEALTH_BAR_PATH, 
        HEALTH_BAR_WIDTH, 
        HEALTH_BAR_HEIGHT, 
        5, 
        (int[]){HEALTH_BAR_FRAMES, HEALTH_BAR_FRAMES, HEALTH_BAR_FRAMES, HEALTH_BAR_FRAMES}
    );

    Image_Init(
        &player->idle, 
        PLAYER_IDLE_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        IDLE_FRAME_DELAY, 
        (int[]){IDLE_FRAMES_0, IDLE_FRAMES_1, IDLE_FRAMES_2, IDLE_FRAMES_3}
    );

    Image_Init(
        &player->run, 
        PLAYER_RUN_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        RUN_FRAME_DELAY, 
        (int[]){RUN_FRAMES, RUN_FRAMES, RUN_FRAMES, RUN_FRAMES}
    );
    
    Image_Init(
        &player->walk, 
        PLAYER_WALK_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        WALK_FRAME_DELAY, 
        (int[]){WALK_FRAMES, WALK_FRAMES, WALK_FRAMES, WALK_FRAMES}
    );

    Image_Init(
        &player->idleAttack, 
        PLAYER_IDLE_ATTACK_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        IDLE_ATTACK_FRAME_DELAY, 
        (int[]){IDLE_ATTACK_FRAMES, IDLE_ATTACK_FRAMES, IDLE_ATTACK_FRAMES, IDLE_ATTACK_FRAMES}
    );

    Image_Init(
        &player->walkAttack, 
        PLAYER_WALK_ATTACK_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        WALK_ATTACK_FRAME_DELAY, 
        (int[]){WALK_ATTACK_FRAMES, WALK_ATTACK_FRAMES, WALK_ATTACK_FRAMES, WALK_ATTACK_FRAMES}
    );

    Image_Init(
        &player->runAttack,
        PLAYER_RUN_ATTACK_PATH,
        FRAME_WIDTH,
        FRAME_HEIGHT,
        RUN_ATTACK_FRAME_DELAY,
        (int[]){RUN_ATTACK_FRAMES, RUN_ATTACK_FRAMES, RUN_ATTACK_FRAMES, RUN_ATTACK_FRAMES}
    );

    Image_Init(
        &player->hurt,
        PLAYER_HURT_PATH,
        FRAME_WIDTH,
        FRAME_HEIGHT,
        HURT_FRAME_DELAY,
        (int[]){HURT_FRAMES, HURT_FRAMES, HURT_FRAMES, HURT_FRAMES}
    );

    Image_Init(
        &player->death,
        PLAYER_DEATH_PATH,
        FRAME_WIDTH,
        FRAME_HEIGHT,
        DEATH_FRAME_DELAY,
        (int[]){DEATH_FRAMES, DEATH_FRAMES, DEATH_FRAMES, DEATH_FRAMES}
    );
    
}


void Player_Update(Game *game) {
    Player *player = &game->player;

    int moving = 0;
    int sprinting = 0;
    
    int newX = player->x;
    int newY = player->y;

    Animation *currentAnim = Player_GetCurrentAnimation(&game->player);
    int finished = Animation_Update(currentAnim, game->player.direction);

    if (player->dead) {
        if (finished) game->gameOver = 1;
        player->state = PLAYER_DEATH;
    }
    else if (player->beenHit && finished) player->beenHit = 0;
    else if (player->beenHit) player->state = PLAYER_HURT;
    else if (!player->attacking) {
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) sprinting = 1;

        int speed = sprinting ? PLAYER_SPRINT_SPEED : PLAYER_SPEED;

        if (GetAsyncKeyState('A') & 0x8000)
        {
            newX -= speed;
            player->direction = DIR_LEFT;
            moving = 1;
        }
        else if (GetAsyncKeyState('D') & 0x8000)
        {
            newX += speed;
            player->direction = DIR_RIGHT;
            moving = 1;
        }
        else if (GetAsyncKeyState('W') & 0x8000)
        {
            newY -= speed;
            player->direction = DIR_UP;
            moving = 1;
        }
        else if (GetAsyncKeyState('S') & 0x8000)
        {
            newY += speed;
            player->direction = DIR_DOWN;
            moving = 1;
        }

        if (!Collision_Check(game, newX, player->y, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY)) {
            player->x = newX;
        }
        if (!Collision_Check(game, player->x, newY, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY)) {
            player->y = newY;
        }

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            player->attacking = 1;
            player->attackDirection = player->direction;

            if (player->state == PLAYER_RUN) {
                player->state = PLAYER_RUN_ATTACK;
                player->attackMoveSpeed = PLAYER_RUN_ATTACK_SPEED;
            }
            else if (player->state == PLAYER_WALK) {
                player->state = PLAYER_WALK_ATTACK;
                player->attackMoveSpeed = PLAYER_WALK_ATTACK_SPEED;
            }
            else {
                player->state = PLAYER_IDLE_ATTACK;
                player->attackMoveSpeed = 0;
            }

            Animation *anim = Player_GetCurrentAnimation(player);
            anim->currentFrame = 0;
            anim->frameTimer = 0;
        } else {
            if (!moving) player->state = PLAYER_IDLE;
            else if (sprinting) player->state = PLAYER_RUN;
            else player->state = PLAYER_WALK;
        }

        if (GetAsyncKeyState('E') & 0x0001) {
            Check_Player_Interact(game);
        }
    } else if (player->attacking) {
        if (currentAnim->currentFrame >= PLAYER_START_ATTACK_FRAME && currentAnim->currentFrame <= PLAYER_END_ATTACK_FRAME) Player_CheckAttackHit(game);
        int newX = player->x;
        int newY = player->y;

        if (player->attackDirection == DIR_LEFT) newX -= player->attackMoveSpeed;
        else if (player->attackDirection == DIR_RIGHT) newX += player->attackMoveSpeed;
        else if (player->attackDirection == DIR_UP) newY -= player->attackMoveSpeed;
        else if (player->attackDirection == DIR_DOWN) newY += player->attackMoveSpeed;

        if (!Collision_Check(game, newX, player->y, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY)) player->x = newX;
        if (!Collision_Check(game, player->x, newY, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY)) player->y = newY;
    }

    HealthBar_Update(&game->player);
    Check_Player_Spawn(game);


    

    if (game->player.attacking && finished) {
        game->player.attacking = 0;
        game->player.state = PLAYER_IDLE;
    } 
}


void Player_Render(Player *player, Game *game, HDC hdc, HDC bufferDC) {
    Animation *healthBar = &game->player.healthBar;

    int healthFrame = (MAX_HEALTH - player->health) * player->healthBar.frameCount[0] / MAX_HEALTH;

    if (healthFrame < 0) healthFrame = 0;
    if (healthFrame >= player->healthBar.frameCount[0])
        healthFrame = player->healthBar.frameCount[0] - 1;

    int healthSrcX = healthFrame * HEALTH_BAR_WIDTH;
    int healthSrcY = 0;

    HDC healthDC = CreateCompatibleDC(hdc);
    SelectObject(healthDC, healthBar->image);

    TransparentBlt(
        bufferDC,
        20,
        20,
        healthBar->frameWidth,
        healthBar->frameHeight,
        healthDC,
        healthSrcX,
        healthSrcY,
        healthBar->frameWidth,
        healthBar->frameHeight,
        RGB(0, 0, 0)
    );
    DeleteDC(healthDC);

    if (game->gameOver) return;
    Animation *currentAnim = Player_GetCurrentAnimation(player);

    int playerScreenX = player->x - game->camera.x;
    int playerScreenY = player->y - game->camera.y;

    HDC spriteDC = CreateCompatibleDC(hdc);
    SelectObject(spriteDC, currentAnim->image);

    int srcX = currentAnim->currentFrame * currentAnim->frameWidth;
    int srcY = player->direction * currentAnim->frameHeight;
    
    /*Rectangle(
        bufferDC,
        playerScreenX + player->hitboxOffsetX,
        playerScreenY + player->hitboxOffsetY,
        playerScreenX + player->hitboxOffsetX + player->hitboxWidth,
        playerScreenY + player->hitboxOffsetY + player->hitboxHeight
    ); // DRAW HITBOX FOR TESTING*/

    TransparentBlt(
        bufferDC,
        playerScreenX,
        playerScreenY,
        player->spriteWidth,
        player->spriteHeight,
        spriteDC,
        srcX,
        srcY,
        player->spriteWidth,
        player->spriteHeight,
        RGB(0, 0, 0)
    );
    DeleteDC(spriteDC);
}

static void Check_Player_Spawn(Game *game) {
    Player *player = &game->player;
    Level *level = &game->levels[game->currentLevel];
    for (int i = 0; i < level->spawnCount; i++) {
        Spawn *spawn = &level->spawns[i];
        if (spawn->remove) continue;
        if (Check_Distance_Range(player->x, player->y, player->spriteWidth, player->spriteHeight,
                                  spawn->x, spawn->y, TILE_SIZE, TILE_SIZE,
                                  PLAYER_INTERACT_RANGE)) {
            Apply_Spawn_Effect(game, spawn);
            spawn->remove = 1;
        }
    }
}


static void Check_Player_Interact(Game *game) {
    Player *player = &game->player; 
    Level *level = &game->levels[game->currentLevel];

    if (level->tiles[level->goalIndex] == TILE_GOAL_OPEN) {
        if (RectsOverlap(
            player->x,
            player->y,
            player->spriteWidth,
            player->spriteHeight,
            (level->goalIndex % level->width) * TILE_SIZE,
            (level->goalIndex / level->width) * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE
        )) {
            Game_Next_Level(game);
        }
    }
}



static void Player_CheckAttackHit(Game *game) {
    Player *player = &game->player;
    Level *level = &game->levels[game->currentLevel];

    if (!player->attacking)
        return;

    int attackX = player->x + player->hitboxOffsetX;
    int attackY = player->y + player->hitboxOffsetY;
    int attackW = player->hitboxWidth;
    int attackH = player->hitboxHeight;

    if (player->direction == DIR_LEFT)
        attackX -= player->hitboxWidth;
    else if (player->direction == DIR_RIGHT)
        attackX += player->hitboxWidth;
    else if (player->direction == DIR_UP)
        attackY -= player->hitboxHeight;
    else if (player->direction == DIR_DOWN)
        attackY += player->hitboxHeight;

    for (int i = 0; i < level->enemyCount; i++)
    {
        Enemy *enemy = &level->enemies[i];
        if (enemy->beenHit || enemy->dead) continue;

        int enemyX = enemy->x + enemy->hitboxOffsetX;
        int enemyY = enemy->y + enemy->hitboxOffsetY;

        if (RectsOverlap(attackX, attackY, attackW, attackH,
                         enemyX, enemyY, enemy->hitboxWidth, enemy->hitboxHeight)) {
            int health = enemy->health - player->attackDamage;
            enemy->health = (health > 0) ? health : 0;
            enemy->beenHit = 1;
            enemy->state = ENEMY_HURT;
            enemy->hurt.currentFrame = 0;
            enemy->hurt.frameTimer = 0;
            
            Enemy_Start_Knockback(game, enemy);
            if (enemy->health == 0) enemy->dead = 1;
            
        }
    }
}

static Animation *Player_GetCurrentAnimation(Player *player) {
    switch (player->state) 
    {
        case PLAYER_IDLE:        return &player->idle;
        case PLAYER_WALK:        return &player->walk;
        case PLAYER_RUN:         return &player->run;
        case PLAYER_IDLE_ATTACK: return &player->idleAttack;
        case PLAYER_WALK_ATTACK: return &player->walkAttack;
        case PLAYER_RUN_ATTACK:  return &player->runAttack;
        case PLAYER_HURT:        return &player->hurt;
        case PLAYER_DEATH:       return &player->death;
        default:                 return &player->idle;
    }
}

static void HealthBar_Update(Player *player) {
    if (player->health <= 20) {
        player->healthBar.currentFrame = 5;
    } else if (player->health <= 40) {
        player->healthBar.currentFrame = 4;
    } else if (player->health <= 60) {
        player->healthBar.currentFrame = 3;
    } else if (player->health <= 80) {
        player->healthBar.currentFrame = 2;
    } else if (player->health < MAX_HEALTH) {
        player->healthBar.currentFrame = 1;
    } else {
        player->healthBar.currentFrame = 0;
    }
}

