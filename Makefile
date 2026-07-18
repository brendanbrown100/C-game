# COMPILE WITH: mingw32-make
CC = gcc

CFLAGS = -Wall -Wextra -Iinclude
LIBS = -lgdi32 -lxinput9_1_0 -lmsimg32 #-mwindows

SRC = src/main.c \
      src/game.c \
      src/player.c \
      src/enemy.c \
      src/window.c \
      src/handler.c \
      src/home.c \
      src/carousel.c \
      src/cannon.c \
      src/pause.c \
      src/settings.c

TARGET = game.exe

all:
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	del $(TARGET)