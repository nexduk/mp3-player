// config.h - every pin and tunable lives here, so wiring changes happen in ONE place.
#pragma once

// ---- SD card (SPI0) ------------------------------------------------------
// NOTE: the SD library reads its pins from hw_config.c, so keep these in sync.
#define PIN_SD_MISO   0
#define PIN_SD_CS     1
#define PIN_SD_SCK    2
#define PIN_SD_MOSI   3

// ---- PCM5102 I2S DAC -----------------------------------------------------
#define PIN_I2S_BCK   6   // LRCK is automatically BCK+1 = GP7
#define PIN_I2S_LRCK  7
#define PIN_I2S_DIN   8

// ---- Audio buffering -----------------------------------------------------
#define AUDIO_FRAMES_PER_BUFFER  1152   // one MPEG-1 Layer III frame
#define AUDIO_BUFFER_COUNT       8      // ~200 ms of cushion against SD hiccups

// ---- Clocks --------------------------------------------------------------
// Software-float MP3 decoding is heavy on the RP2040. 250 MHz is a common
// safe overclock; set to 0 to stay at the stock 125 MHz.
#define OVERCLOCK_KHZ  250000