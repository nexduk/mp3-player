#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/audio.h"
#include "pico/audio_i2s.h"
#include "hardware/dma.h"

#include "config.h"
#include "audio_out.h"

static audio_buffer_pool_t *pool = NULL;
static audio_buffer_t *cur = NULL;   // buffer currently being filled
static uint cur_fill = 0;            // frames written into it

static audio_format_t audio_format = {
    .sample_freq = 44100,
    .pcm_format = AUDIO_PCM_FORMAT_S32,
    .channel_count = AUDIO_CHANNEL_STEREO,
};

static audio_buffer_format_t producer_format = {
    .format = &audio_format,
    .sample_stride = 8,              // 2 channels * 4 bytes
};

static audio_i2s_config_t i2s_config = {
    .data_pin = PIN_I2S_DIN,
    .clock_pin_base = PIN_I2S_BCK,
    .dma_channel0 = 0,   // overwritten in audio_out_init()
    .dma_channel1 = 1,
    .pio_sm = 0,
};

bool audio_out_init(uint32_t sample_rate) {
    audio_format.sample_freq = sample_rate;

    // The SD driver grabs DMA channels too, and this library claims the exact
    // numbers it is given. So find two free ones, release them, and pass those.
    int c0 = dma_claim_unused_channel(true);
    int c1 = dma_claim_unused_channel(true);
    dma_channel_unclaim(c0);
    dma_channel_unclaim(c1);
    i2s_config.dma_channel0 = c0;
    i2s_config.dma_channel1 = c1;

    pool = audio_new_producer_pool(&producer_format, AUDIO_BUFFER_COUNT, AUDIO_FRAMES_PER_BUFFER);
    if (!pool) return false;

    if (!audio_i2s_setup(&audio_format, &audio_format, &i2s_config)) return false;
    if (!audio_i2s_connect(pool)) return false;

    // Prime the pipeline with one silent buffer, then start the DAC.
    audio_buffer_t *ab = take_audio_buffer(pool, true);
    int32_t *s = (int32_t *)ab->buffer->bytes;
    for (uint i = 0; i < ab->max_sample_count * 2; i++) s[i] = 0;
    ab->sample_count = ab->max_sample_count;
    give_audio_buffer(pool, ab);

    audio_i2s_set_enabled(true);
    cur = NULL;
    cur_fill = 0;
    return true;
}

void audio_out_write(const int16_t *pcm, uint frames, uint channels) {
    while (frames) {
        if (!cur) {
            cur = take_audio_buffer(pool, true);   // blocks until DMA frees one
            cur_fill = 0;
        }
        int32_t *dst = (int32_t *)cur->buffer->bytes;
        uint space = cur->max_sample_count - cur_fill;
        uint n = frames < space ? frames : space;

        for (uint i = 0; i < n; i++) {
            int32_t l = pcm[0];
            int32_t r = (channels > 1) ? pcm[1] : l;   // mono -> both channels
            pcm += channels;
            dst[2 * (cur_fill + i)]     = l * 65536;   // 16-bit -> 32-bit full scale
            dst[2 * (cur_fill + i) + 1] = r * 65536;
        }
        cur_fill += n;
        frames -= n;

        if (cur_fill == cur->max_sample_count) {
            cur->sample_count = cur->max_sample_count;
            give_audio_buffer(pool, cur);
            cur = NULL;
        }
    }
}

void audio_out_flush(void) {
    if (!cur) return;
    int32_t *dst = (int32_t *)cur->buffer->bytes;
    for (uint i = cur_fill * 2; i < cur->max_sample_count * 2; i++) dst[i] = 0;
    cur->sample_count = cur->max_sample_count;
    give_audio_buffer(pool, cur);
    cur = NULL;
}

void audio_out_deinit(void) {
    if (!pool) return;
    audio_out_flush();
    sleep_ms(400);                       // let the queued buffers play out

    audio_i2s_set_enabled(false);
    audio_i2s_end();

    // Free every buffer, wherever it is (same approach as the library's sample).
    audio_buffer_t *ab;
    while ((ab = take_audio_buffer(pool, false)) != NULL)     { free(ab->buffer->bytes); free(ab->buffer); }
    while ((ab = get_free_audio_buffer(pool, false)) != NULL) { free(ab->buffer->bytes); free(ab->buffer); }
    while ((ab = get_full_audio_buffer(pool, false)) != NULL) { free(ab->buffer->bytes); free(ab->buffer); }
    free(pool);
    pool = NULL;
}