#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include "window.h"
#include "handler.h"

#define WIDTH  800
#define HEIGHT 600
#define FPS    75
#define TIME_PER_FRAME 0.015f
#define FPS_X  744
#define FPS_Y  10
#define GAME_TITLE "SUPER SHITTY WORLD"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

#endif