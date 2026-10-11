// audio_out.h - thin wrapper around pico_audio_i2s_32b.
// Callers just hand it 16-bit PCM; it converts to 32-bit stereo and queues it.
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "pico/types.h"

bool audio_out_init(uint32_t sample_rate);
// Blocks while the DMA buffers are full (that's what paces the decoder).
void audio_out_write(const int16_t *pcm, uint frames, uint channels);
// Push any partially filled buffer (pads with silence).
void audio_out_flush(void);
// Let queued audio finish playing, then shut the I2S output down.
void audio_out_deinit(void);