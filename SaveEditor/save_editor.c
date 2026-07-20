#include <stdio.h>
#include <stdlib.h>

#define GAME_STATE_PATH "Assets/Data/savegame.dat"

typedef struct GameData {
    int health;
    int score;
    int level;
} GameData;

static void Print_Save(GameData data) {
    printf("\nCurrent Save Data:\n");
    printf("Health: %d\n", data.health);
    printf("Score : %d\n", data.score);
    printf("Level : %d\n", data.level + 1);
}

int main(void) {
    GameData data;
    FILE *file;

    file = fopen(GAME_STATE_PATH, "rb");

    if (file == NULL) {
        printf("No save file found.\n");
        printf("Creating a new save file...\n");

        data.health = 100;
        data.score = 0;
        data.level = 1;
    } else {
        if (fread(&data, sizeof(GameData), 1, file) != 1) {
            printf("Could not read save file correctly.\n");
            fclose(file);
            return 1;
        }

        fclose(file);
    }

    Print_Save(data);

    printf("\nEnter new health: ");
    scanf("%d", &data.health);

    printf("Enter new score: ");
    scanf("%d", &data.score);

    printf("Enter new level: ");
    int level;
    scanf("%d", &level);
    data.level = level - 1;

    file = fopen(GAME_STATE_PATH, "wb");

    if (file == NULL) {
        printf("Could not open save file for writing.\n");
        return 1;
    }

    if (fwrite(&data, sizeof(GameData), 1, file) != 1) {
        printf("Could not write save file correctly.\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    printf("\nSave file updated successfully!\n");
    Print_Save(data);

    return 0;
}