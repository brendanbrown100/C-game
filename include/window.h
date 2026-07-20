#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>

typedef struct Window {
    HWND hwnd;
    
    int width;
    int height;
} Window;

int Window_Create(Window *window, HINSTANCE hInstance, int nCmdShow, int width, int height, const char *title);

#endif