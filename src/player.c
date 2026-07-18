#include "player.h"
#include "game.h"
#include "main.h"
#include <windows.h>
#include <xinput.h>
#include <stdio.h>

static void HealthBar_Update(Player *player);
static void Player_CheckAttackHit(Game *game);
static Animation *Player_GetCurrentAnimation(Player *player);
static void Check_Player_Interact(Game *game);
static void Check_Player_Spawn(Game *game);
static void Check_Player_Coin(Game *game);
static int Check_Enemy_Collision(Game *game, int playerX, int playerY);
static void Player_Start_Dash(Player *player);
static void Player_Update_Dash(Game *game);
static void Throw_Barrel(Game *game);
static void Drop_Barrel(Game *game);
static void Update_Carried_Barrel(Game *game);
static int Cannon_Player_Collision(Game *game, Cannon *cannon, int playerX, int playerY);


void Player_Init(Game *game, Level *level) {
    Player *player = &game->player;

    player->x = level->startX * TILE_SIZE;
    player->y = level->startY * TILE_SIZE;
    player->hitboxWidth = PLAYER_WIDTH;
    player->hitboxHeight = PLAYER_HEIGHT;
    player->hitboxOffsetX = (FRAME_WIDTH - PLAYER_WIDTH) / 2;
    player->hitboxOffsetY = (FRAME_HEIGHT - PLAYER_HEIGHT) / 2;
    player->spriteWidth = FRAME_WIDTH;
    player->spriteHeight = FRAME_HEIGHT;
    if (!player->health) player->health = MAX_HEALTH;
    if (!player->score) player->score = 0;
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
    player->dashing = 0;
    player->dashDirection = DIR_DOWN;
    player->dashTimer = 0;
    player->dashCooldown = 0;
    player->dashWasDown = 0;
    player->hasBarrel = 0;
    player->throwingBarrel = 0;
    player->barrelThrown = 0;
    player->carriedBarrelIndex = -1;
    player->lastMoveKey = DIR_DOWN;
    player->upWasDown = 0;
    player->leftWasDown = 0;
    player->downWasDown = 0;
    player->rightWasDown = 0;

    Image_Init(
        &player->healthBar, 
        HEALTH_BAR_PATH, 
        HEALTH_BAR_WIDTH, 
        HEALTH_BAR_HEIGHT, 
        5, 
        (int[]){HEALTH_BAR_FRAMES, HEALTH_BAR_FRAMES, HEALTH_BAR_FRAMES, HEALTH_BAR_FRAMES}
    );

    Image_Init(
        &player->barrelIdle, 
        PLAYER_BARREL_IDLE_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        BARREL_IDLE_FRAME_DELAY, 
        (int[]){BARREL_IDLE_FRAMES_0, BARREL_IDLE_FRAMES_1, BARREL_IDLE_FRAMES_2, BARREL_IDLE_FRAMES_3}
    );

    Image_Init(
        &player->barrelRun, 
        PLAYER_BARREL_RUN_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        BARREL_RUN_FRAME_DELAY, 
        (int[]){BARREL_RUN_FRAMES, BARREL_RUN_FRAMES, BARREL_RUN_FRAMES, BARREL_RUN_FRAMES}
    );
    
    Image_Init(
        &player->barrelWalk, 
        PLAYER_BARREL_WALK_PATH, 
        FRAME_WIDTH, 
        FRAME_HEIGHT, 
        BARREL_WALK_FRAME_DELAY, 
        (int[]){BARREL_WALK_FRAMES, BARREL_WALK_FRAMES, BARREL_WALK_FRAMES, BARREL_WALK_FRAMES}
    );

    Image_Init(
        &player->barrelThrowIdle,
        PLAYER_BARREL_THROW_IDLE_PATH,
        FRAME_WIDTH,
        FRAME_HEIGHT,
        BARREL_THROW_IDLE_FRAME_DELAY,
        (int[]){
            BARREL_THROW_IDLE_FRAMES,
            BARREL_THROW_IDLE_FRAMES,
            BARREL_THROW_IDLE_FRAMES,
            BARREL_THROW_IDLE_FRAMES
        }
    );

    Image_Init(
        &player->barrelThrowWalk,
        PLAYER_BARREL_THROW_WALK_PATH,
        FRAME_WIDTH,
        FRAME_HEIGHT,
        BARREL_THROW_WALK_FRAME_DELAY,
        (int[]){
            BARREL_THROW_WALK_ATTACK_FRAMES,
            BARREL_THROW_WALK_ATTACK_FRAMES,
            BARREL_THROW_WALK_ATTACK_FRAMES,
            BARREL_THROW_WALK_ATTACK_FRAMES
        }
    );

    Image_Init(
        &player->barrelThrowRun,
        PLAYER_BARREL_THROW_RUN_PATH,
        FRAME_WIDTH,
        FRAME_HEIGHT,
        BARREL_THROW_RUN_FRAME_DELAY,
        (int[]){
            BARREL_THROW_RUN_ATTACK_FRAMES,
            BARREL_THROW_RUN_ATTACK_FRAMES,
            BARREL_THROW_RUN_ATTACK_FRAMES,
            BARREL_THROW_RUN_ATTACK_FRAMES
        }
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

    Image_Init(
        &player->dash,
        PLAYER_DASH_PATH,
        FRAME_WIDTH,
        FRAME_HEIGHT,
        DASH_FRAME_DELAY,
        (int[]){
            DASH_FRAMES,
            DASH_FRAMES,
            DASH_FRAMES,
            DASH_FRAMES
        }
    );
    
}


void Player_Update(Game *game) {
    Player *player = &game->player;

    int moving = 0;
    int sprinting = 0;
    
    int newX = player->x;
    int newY = player->y;

    int dashIsDown =
        (GetAsyncKeyState(game->dashKeyCode) & 0x8000) != 0;

    int dashPressed =
        dashIsDown && !player->dashWasDown;

    if (player->dashCooldown > 0) {
        player->dashCooldown--;
    }

    if (player->dead) {
        player->state = PLAYER_DEATH;
    }
    else if (player->beenHit) {
        player->state = PLAYER_HURT;
    }
    else if (dashPressed) {
        Player_Start_Dash(player);
    }

    player->dashWasDown = dashIsDown;


    if (player->dashing) {
        player->state = PLAYER_DASH;
        player->direction = player->dashDirection;

        Animation_Update(
            &player->dash,
            player->dashDirection
        );

        Player_Update_Dash(game);

        goto skip_collision;
    }

    Animation *currentAnim = Player_GetCurrentAnimation(&game->player);
    int finished = Animation_Update(currentAnim, game->player.direction);

    if (player->dead) {
        if (finished) {
            game->gameOverAnim.currentFrame = 0;
            game->gameOver = 1;
        }
        player->state = PLAYER_DEATH;
    } else if (player->beenHit && finished) {
        player->beenHit = 0;
        player->hurt.currentFrame = 0;
    } else if (player->beenHit) {
        if (player->hasBarrel) {
            Drop_Barrel(game);
        }

        player->state = PLAYER_HURT;
    } else if (!player->attacking && !player->throwingBarrel) {
        if ((GetAsyncKeyState(game->sprintKeyCode) & 0x8000)) sprinting = 1;

        int speed = sprinting ? PLAYER_SPRINT_SPEED : PLAYER_SPEED;

        int upDown = (GetAsyncKeyState(game->upKeyCode) & 0x8000) != 0;
        int leftDown = (GetAsyncKeyState(game->leftKeyCode) & 0x8000) != 0;
        int downDown = (GetAsyncKeyState(game->downKeyCode) & 0x8000) != 0;
        int rightDown = (GetAsyncKeyState(game->rightKeyCode) & 0x8000) != 0;

        /*
        * If a key became pressed this frame, remember it as the newest direction.
        */
        if (upDown && !player->upWasDown) {
            player->lastMoveKey = DIR_UP;
        }
        if (leftDown && !player->leftWasDown) {
            player->lastMoveKey = DIR_LEFT;
        }
        if (downDown && !player->downWasDown) {
            player->lastMoveKey = DIR_DOWN;
        }
        if (rightDown && !player->rightWasDown) {
            player->lastMoveKey = DIR_RIGHT;
        }

        /*
        * If the last pressed key is still held, move that way.
        * If not, fall back to any other held movement key.
        */
        if (player->lastMoveKey == DIR_UP && upDown) {
            newY -= speed;
            player->direction = DIR_UP;
            moving = 1;
        }
        else if (player->lastMoveKey == DIR_LEFT && leftDown) {
            newX -= speed;
            player->direction = DIR_LEFT;
            moving = 1;
        }
        else if (player->lastMoveKey == DIR_DOWN && downDown) {
            newY += speed;
            player->direction = DIR_DOWN;
            moving = 1;
        }
        else if (player->lastMoveKey == DIR_RIGHT && rightDown) {
            newX += speed;
            player->direction = DIR_RIGHT;
            moving = 1;
        }
        else {
            /*
            * Fallback if the last pressed key was released but another key is still held.
            */
            if (upDown) {
                newY -= speed;
                player->direction = DIR_UP;
                player->lastMoveKey = DIR_UP;
                moving = 1;
            }
            else if (leftDown) {
                newX -= speed;
                player->direction = DIR_LEFT;
                player->lastMoveKey = DIR_LEFT;
                moving = 1;
            }
            else if (downDown) {
                newY += speed;
                player->direction = DIR_DOWN;
                player->lastMoveKey = DIR_DOWN;
                moving = 1;
            }
            else if (rightDown) {
                newX += speed;
                player->direction = DIR_RIGHT;
                player->lastMoveKey = DIR_RIGHT;
                moving = 1;
            }
        }

        /*
        * Save key states for next frame.
        */
        player->upWasDown = upDown;
        player->leftWasDown = leftDown;
        player->downWasDown = downDown;
        player->rightWasDown = rightDown;

        if (!moving) goto skipCollisionCheck;
        if (!Collision_Check(game, newX, player->y, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY) && 
            !Check_Enemy_Collision(game, newX, player->y)) {
            player->x = newX;
        }

        if (!Collision_Check(game, player->x, newY, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY) &&
            !Check_Enemy_Collision(game, player->x, newY)) {
            player->y = newY;
        }

        skipCollisionCheck:

        if (GetAsyncKeyState(game->attackKeyCode) & 0x8000) {
            if (!player->hasBarrel) {
                player->attacking = 1;
            } else {
                player->throwingBarrel = 1;
                player->barrelThrown = 0;
            }
            player->attackDirection = player->direction;

            if (player->state == PLAYER_RUN || player->state == PLAYER_BARREL_RUN) {
                player->state = (!player->hasBarrel) ? PLAYER_RUN_ATTACK : PLAYER_BARREL_THROW_RUN;
                player->attackMoveSpeed = PLAYER_RUN_ATTACK_SPEED;
            }
            else if (player->state == PLAYER_WALK || player->state == PLAYER_BARREL_WALK) {
                player->state = (!player->hasBarrel) ? PLAYER_WALK_ATTACK : PLAYER_BARREL_THROW_WALK;
                player->attackMoveSpeed = PLAYER_WALK_ATTACK_SPEED;
            }
            else {
                player->state = (!player->hasBarrel) ? PLAYER_IDLE_ATTACK : PLAYER_BARREL_THROW_IDLE;
                player->attackMoveSpeed = 0;
            }

            Animation *anim = Player_GetCurrentAnimation(player);
            anim->currentFrame = 0;
            anim->frameTimer = 0;
        } else {
            if (!moving) player->state = (player->hasBarrel) ? PLAYER_BARREL_IDLE : PLAYER_IDLE;
            else if (sprinting) player->state = (player->hasBarrel) ? PLAYER_BARREL_RUN : PLAYER_RUN;
            else player->state = (player->hasBarrel) ? PLAYER_BARREL_WALK : PLAYER_WALK;
        }

        if (GetAsyncKeyState(game->interactKeyCode) & 0x0001) {
            Check_Player_Interact(game);
        }
    } else if (player->attacking || player->throwingBarrel) {
        if (player->attacking) {
            if (currentAnim->currentFrame >= PLAYER_START_ATTACK_FRAME && currentAnim->currentFrame <= PLAYER_END_ATTACK_FRAME) Player_CheckAttackHit(game);
        } else if (player->throwingBarrel) {
            if (currentAnim->currentFrame == BARREL_THROW_RELEASE_FRAME && !player->barrelThrown) {
                player->barrelThrown = 1;
                Throw_Barrel(game);
            }
        }
        
        int newX = player->x;
        int newY = player->y;

        if (player->attackDirection == DIR_LEFT) newX -= player->attackMoveSpeed;
        else if (player->attackDirection == DIR_RIGHT) newX += player->attackMoveSpeed;
        else if (player->attackDirection == DIR_UP) newY -= player->attackMoveSpeed;
        else if (player->attackDirection == DIR_DOWN) newY += player->attackMoveSpeed;

        if (!Collision_Check(game, newX, player->y, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY) &&
            !Check_Enemy_Collision(game, newX, player->y)) {
            player->x = newX;
        }

        if (!Collision_Check(game, player->x, newY, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY) &&
            !Check_Enemy_Collision(game, player->x, newY)) {
            player->y = newY;
        }
    }

    int fallCheck = Check_Fall(game, player->x, player->y, player->hitboxWidth, player->hitboxHeight, player->hitboxOffsetX, player->hitboxOffsetY);
    if (fallCheck == 1) {
        player->health = 0;
        player->dead = 1;
    } else if (fallCheck == 2) {
        Spawn_Jet(game);
    }
    skip_collision:

    
    HealthBar_Update(&game->player);
    Check_Player_Spawn(game);
    Check_Player_Coin(game);
    Update_Carried_Barrel(game);


    

    if (player->attacking && finished) {
        player->attacking = 0;
        player->state = PLAYER_IDLE;
    } else if (player->throwingBarrel && finished) {
        player->throwingBarrel = 0;
        player->barrelThrown = 0;
        player->hasBarrel = 0;
        player->state = PLAYER_IDLE;
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
        4,
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

static void Throw_Barrel(Game *game) {
    Player *player = &game->player;

    if (!player->hasBarrel ||
        player->carriedBarrelIndex < 0 ||
        player->carriedBarrelIndex >= game->barrelCount) {
        return;
    }

    Barrel *barrel =
        &game->barrels[player->carriedBarrelIndex];

    barrel->pickedUp = 0;
    barrel->thrown = 1;
    barrel->destroyed = 0;
    barrel->remove = 0;

    barrel->dir = player->direction;
    barrel->isVerticle =
        player->direction == DIR_LEFT ||
        player->direction == DIR_RIGHT;

    barrel->speed = BARREL_SPEED;
    barrel->frame = 0;
    barrel->frameDelay = BARREL_FRAME_DELAY;

    barrel->x = player->x + player->hitboxOffsetX;
    barrel->y = player->y + player->hitboxOffsetY;

    player->hasBarrel = 0;
    player->carriedBarrelIndex = -1;
}

static void Player_Start_Dash(Player *player) {
    if (player->dashing ||
        player->dashCooldown > 0 ||
        player->dead ||
        player->beenHit ||
        player->attacking) {
        return;
    }

    player->dashing = 1;
    player->state = PLAYER_DASH;

    player->dashDirection = player->direction;
    player->dashTimer = PLAYER_DASH_DURATION;
    player->dashCooldown = PLAYER_DASH_COOLDOWN;

    /*
     * The player's current position is the first safe position.
     */
    player->dashSafeX = player->x;
    player->dashSafeY = player->y;

    player->dash.currentFrame = 0;
    player->dash.frameTimer = 0;
}

static void Player_Update_Dash(Game *game) {
    Player *player = &game->player;

    for (int step = 0; step < PLAYER_DASH_SPEED; step++) {
        int newX = player->x;
        int newY = player->y;

        switch (player->dashDirection) {
            case DIR_LEFT:
                newX--;
                break;

            case DIR_RIGHT:
                newX++;
                break;

            case DIR_UP:
                newY--;
                break;

            case DIR_DOWN:
                newY++;
                break;
        }

        /*
         * Walls still block the dash.
         */
        if (Collision_Check(
                game,
                newX,
                newY,
                player->hitboxWidth,
                player->hitboxHeight,
                player->hitboxOffsetX,
                player->hitboxOffsetY)) {

            player->dashTimer = 0;
            break;
        }

        /*
         * Move through the enemy regardless.
         */
        player->x = newX;
        player->y = newY;

        /*
         * Remember this position only when it is not inside an enemy.
         *
         * We are checking enemy collision here, but we are not using
         * it to stop the dash.
         */
        if (!Check_Enemy_Collision(
                game,
                player->x,
                player->y)) {

            player->dashSafeX = player->x;
            player->dashSafeY = player->y;
        }
    }

    if (player->dashTimer > 0) {
        player->dashTimer--;
    }

    if (player->dashTimer <= 0) {
        /*
         * Do not allow the dash to finish inside an enemy.
         */
        if (Check_Enemy_Collision(
                game,
                player->x,
                player->y)) {

            player->x = player->dashSafeX;
            player->y = player->dashSafeY;
        }

        player->dashing = 0;
        player->state = PLAYER_IDLE;

        player->dash.currentFrame = 0;
        player->dash.frameTimer = 0;
    }
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

static void Check_Player_Coin(Game *game) {
    Player *player = &game->player;
    int playerX = player->x + player->hitboxOffsetX;
    int playerY = player->y + player->hitboxOffsetY;
    for (int i = 0; i < game->coinCount; i++) {
        Coin *coin = &game->coins[i];
        if (coin->remove) continue;

        int coinX = coin->x + COIN_HITBOX_OFFSET_X;
        int coinY = coin->y + COIN_HITBOX_OFFSET_Y;

        if (RectsOverlap(playerX, playerY, player->hitboxWidth, player->hitboxHeight,
                         coinX, coinY, COIN_HITBOX_WIDTH, COIN_HITBOX_HEIGHT)) {
            player->score += coin->value;
            coin->remove = 1;
        }
    }
}

static void Check_Player_Interact(Game *game) {
    Player *player = &game->player; 
    Level *level = &game->levels[game->currentLevel];

    if (level->tiles[level->goalIndex] == TILE_GOAL_OPEN) {
        if (RectsOverlap(
            player->x + player->hitboxOffsetX,
            player->y + player->hitboxOffsetY,
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

    if (player->hasBarrel) {
        return;
    }

    for (int i = 0; i < game->barrelCount; i++) {
        Barrel *barrel = &game->barrels[i];

        if (barrel->remove ||
            barrel->destroyed ||
            barrel->pickedUp ||
            barrel->thrown) {
            continue;
        }

        if (RectsOverlap(
                player->x + player->hitboxOffsetX,
                player->y + player->hitboxOffsetY,
                player->hitboxWidth,
                player->hitboxHeight,
                barrel->x,
                barrel->y,
                BARREL_WIDTH,
                BARREL_HEIGHT)) {

            barrel->pickedUp = 1;

            player->hasBarrel = 1;
            player->carriedBarrelIndex = i;

            return;
        }
    }
}

static void Update_Carried_Barrel(Game *game) {
    Player *player = &game->player;

    if (!player->hasBarrel ||
        player->carriedBarrelIndex < 0 ||
        player->carriedBarrelIndex >= game->barrelCount) {
        return;
    }

    Barrel *barrel =
        &game->barrels[player->carriedBarrelIndex];

    if (barrel->remove) {
        player->hasBarrel = 0;
        player->carriedBarrelIndex = -1;
        return;
    }

    barrel->x = player->x + 16;
    barrel->y = player->y + 8;

    barrel->pickedUp = 1;
    barrel->thrown = 0;
}

static void Drop_Barrel(Game *game) {
    Player *player = &game->player;

    if (!player->hasBarrel ||
        player->carriedBarrelIndex < 0 ||
        player->carriedBarrelIndex >= game->barrelCount) {
        return;
    }

    Barrel *barrel =
        &game->barrels[player->carriedBarrelIndex];

    barrel->pickedUp = 0;
    barrel->thrown = 0;
    barrel->destroyed = 0;
    barrel->remove = 0;

    barrel->x = player->x + player->hitboxOffsetX;
    barrel->y = player->y + player->hitboxOffsetY;

    player->hasBarrel = 0;
    player->throwingBarrel = 0;
    player->barrelThrown = 0;
    player->carriedBarrelIndex = -1;
}



static void Player_CheckAttackHit(Game *game) {
    Player *player = &game->player;
    Level *level = &game->levels[game->currentLevel];

    if (!player->attacking)
        return;

    RECT attackBox;
    Get_Attack_Box(
        player->x, player->y,
        player->hitboxOffsetX, player->hitboxOffsetY,
        player->hitboxWidth, player->hitboxHeight,
        player->direction, PLAYER_ATTACK_RANGE,
        &attackBox
    );

    for (int i = 0; i < level->enemyCount; i++) {
        Enemy *enemy = &level->enemies[i];

        if (enemy->dead || enemy->remove || enemy->beenHit) continue;

        RECT enemyBox = {
            enemy->x + enemy->hitboxOffsetX,
            enemy->y + enemy->hitboxOffsetY,
            enemy->x + enemy->hitboxOffsetX + enemy->hitboxWidth,
            enemy->y + enemy->hitboxOffsetY + enemy->hitboxHeight
        };

        if (Rect_Overlap(attackBox, enemyBox)) {
            enemy->beenHit = 1;
            enemy->state = ENEMY_HURT;
            enemy->attacking = 0;
            enemy->attackHit = 0;
            Enemy_Start_Knockback(game, enemy);
            int health = enemy->health - player->attackDamage;
            if (health <= 0) enemy->dead = 1;
            enemy->health = (health > 0) ? health : 0;
        }
    }
}

static int Check_Enemy_Collision(Game *game, int playerX, int playerY) {
    Level *level = &game->levels[game->currentLevel];

    playerX += game->player.hitboxOffsetX;
    playerY += game->player.hitboxOffsetY;

    for (int i = 0; i < level->enemyCount; i++) {
        Enemy *enemy = &level->enemies[i];
        if (enemy->remove) continue;

        int enemyX = enemy->x + enemy->hitboxOffsetX;
        int enemyY = enemy->y + enemy->hitboxOffsetY;

        if (RectsOverlap(playerX, playerY,
                        game->player.hitboxWidth, game->player.hitboxHeight,
                        enemyX, enemyY, enemy->hitboxWidth, enemy->hitboxHeight)) {
                            return 1;
        }
    }

    for (int i = 0; i < game->cannonCount; i++) {
        Cannon *cannon = &game->cannons[i];
        if (cannon->remove) continue;
        if (Cannon_Player_Collision(game, cannon, playerX, playerY)) return 1;
    }

    return 0;
}

static int Cannon_Player_Collision(Game *game, Cannon *cannon, int playerX, int playerY) {
    Player *player = &game->player;

    playerX += player->hitboxOffsetX;
    playerY += player->hitboxOffsetY;

    if (RectsOverlap(playerX, playerY, player->hitboxWidth, player->hitboxHeight, cannon->x, cannon->y, CANNON_FRAME_WIDTH, CANNON_FRAME_HEIGHT)) {
        return 1;
    }
    return 0;
}

static Animation *Player_GetCurrentAnimation(Player *player) {
    switch (player->state) 
    {
        case PLAYER_IDLE:              return &player->idle;
        case PLAYER_WALK:              return &player->walk;
        case PLAYER_RUN:               return &player->run;
        case PLAYER_IDLE_ATTACK:       return &player->idleAttack;
        case PLAYER_WALK_ATTACK:       return &player->walkAttack;
        case PLAYER_RUN_ATTACK:        return &player->runAttack;
        case PLAYER_DASH:              return &player->dash;
        case PLAYER_BARREL_IDLE:       return &player->barrelIdle;
        case PLAYER_BARREL_WALK:       return &player->barrelWalk;
        case PLAYER_BARREL_RUN:        return &player->barrelRun;
        case PLAYER_BARREL_THROW_IDLE: return &player->barrelThrowIdle;
        case PLAYER_BARREL_THROW_WALK: return &player->barrelThrowWalk;
        case PLAYER_BARREL_THROW_RUN:  return &player->barrelThrowRun;        case PLAYER_HURT:         return &player->hurt;
        case PLAYER_DEATH:             return &player->death;
        default:                       return &player->idle;
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