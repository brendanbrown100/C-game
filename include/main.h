#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include "window.h"
#include "handler.h"

#define WIDTH  810
#define HEIGHT 610
#define FPS    32
#define TIME_PER_FRAME 0.03125
#define FPS_X  750
#define FPS_Y  10
#define GAME_TITLE "Super Shitty World"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

#endif