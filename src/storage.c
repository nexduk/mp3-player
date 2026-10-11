#include <stdio.h>
#include "ff.h"
#include "f_util.h"
#include "storage.h"

static FATFS fs;

bool storage_mount(void) {
    FRESULT fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("SD mount failed: %s (%d)\n", FRESULT_str(fr), fr);
        return false;
    }
    puts("SD mounted");
    return true;
}

void storage_list_root(void) {
    DIR dir;
    FILINFO fno;
    if (f_opendir(&dir, "0:/") != FR_OK) {
        puts("Could not open root directory");
        return;
    }
    puts("Files on card:");
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
        printf("  %s%s  (%lu bytes)\n", fno.fname, (fno.fattrib & AM_DIR) ? "/" : "", (unsigned long)fno.fsize);
    }
    f_closedir(&dir);
}