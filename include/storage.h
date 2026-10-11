// storage.h - SD card mounting and simple file helpers.
#pragma once
#include <stdbool.h>

bool storage_mount(void);
void storage_list_root(void);   // handy for checking the card is readable