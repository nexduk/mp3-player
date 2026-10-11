// mp3_player.h - decode an MP3 from the SD card and play it through audio_out.
// The decoder (minimp3) is hidden in the .c file, so it can be swapped later
// (e.g. for Helix) without touching anything else.
#pragma once
#include <stdbool.h>

// Blocks until the file has finished playing. Returns false on error.
bool mp3_play_file(const char *path);