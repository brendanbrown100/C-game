#include <stdio.h>
#include <windows.h>

int main(void) {
    printf("Listening for input...\n");
    printf("Press ESC to exit.\n\n");

    while (1) {
        for (int virtualCode = 1; virtualCode < 256; virtualCode++) {
            if (GetAsyncKeyState(virtualCode) & 1) {
                printf("Virtual key code: %d | Hex: 0x%02X\n",
                       virtualCode,
                       virtualCode);

                if (virtualCode == VK_ESCAPE) {
                    printf("Exiting...\n");
                    return 0;
                }
            }
        }

        Sleep(10);
    }
}