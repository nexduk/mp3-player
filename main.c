#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include "config.h"
#include "storage.h"
#include "mp3_player.h"

#include "ff.h"

FATFS fs;

bool init_storage() {
    FRESULT fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("Mount failed: error code %d\n", fr);
        return false;
    }
    printf("SD card mounted successfully.\n");
    return true;
}

int main() {
    stdio_init_all();
    
    // Give the USB serial connection a moment to start
    sleep_ms(2000);

    printf("Starting system...\n");

    // Initialize the SD card and FatFS
    if (!init_storage()) {
        printf("Halting due to storage initialization failure.\n");
        while (true) {
            tight_loop_contents();
        }
    }

    // Your application logic goes here
    while (true) {
        tight_loop_contents();
    }
}