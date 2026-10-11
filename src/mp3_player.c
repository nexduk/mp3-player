#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "ff.h"
#include "f_util.h"

#include "audio_out.h"
#include "mp3_player.h"

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_ONLY_MP3
#include "minimp3.h"

#define IN_BUF_SIZE     (16 * 1024)
#define REFILL_BELOW    (8 * 1024)
#define STATS_EVERY     200          // print decode load every N frames

static uint8_t inbuf[IN_BUF_SIZE];
static int16_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
static mp3dec_t dec;

// Skip an ID3v2 tag at the start of the file, if present.
static void skip_id3v2(FIL *f) {
    uint8_t h[10];
    UINT br;
    if (f_read(f, h, 10, &br) == FR_OK && br == 10 && memcmp(h, "ID3", 3) == 0) {
        uint32_t size = ((h[6] & 0x7f) << 21) | ((h[7] & 0x7f) << 14) |
                        ((h[8] & 0x7f) << 7)  |  (h[9] & 0x7f);
        size += 10 + ((h[5] & 0x10) ? 10 : 0);   // header (+ footer if flagged)
        f_lseek(f, size);
    } else {
        f_lseek(f, 0);
    }
}

bool mp3_play_file(const char *path) {
    FIL f;
    FRESULT fr = f_open(&f, path, FA_READ);
    if (fr != FR_OK) {
        printf("Cannot open %s: %s (%d)\n", path, FRESULT_str(fr), fr);
        return false;
    }
    skip_id3v2(&f);
    mp3dec_init(&dec);

    size_t pos = 0, len = 0;
    bool eof = false, audio_on = false;
    uint32_t frames = 0;
    uint64_t dec_us = 0, play_us = 0;

    for (;;) {
        // Keep the input buffer topped up.
        if (!eof && len - pos < REFILL_BELOW) {
            memmove(inbuf, inbuf + pos, len - pos);
            len -= pos;
            pos = 0;
            UINT want = IN_BUF_SIZE - len, br = 0;
            fr = f_read(&f, inbuf + len, want, &br);
            if (fr != FR_OK) {
                printf("Read error: %s (%d)\n", FRESULT_str(fr), fr);
                break;
            }
            if (br < want) eof = true;
            len += br;
        }
        if (len == pos) break;   // nothing left

        mp3dec_frame_info_t info;
        uint64_t t0 = time_us_64();
        int n = mp3dec_decode_frame(&dec, inbuf + pos, (int)(len - pos), pcm, &info);
        uint64_t t1 = time_us_64();

        if (info.frame_bytes == 0) {   // no frame found
            if (eof) break;
            pos = len;                 // junk: drop the buffer and refill
            continue;
        }
        pos += info.frame_bytes;
        if (n == 0) continue;          // skipped a tag / garbage, no audio

        if (!audio_on) {
            printf("MP3: %d Hz, %d ch, %d kbps\n", info.hz, info.channels, info.bitrate_kbps);
            if (!audio_out_init(info.hz)) {
                puts("audio_out_init failed");
                break;
            }
            audio_on = true;
        }

        audio_out_write(pcm, n, info.channels);

        dec_us += t1 - t0;
        play_us += (uint64_t)n * 1000000u / info.hz;
        if (++frames % STATS_EVERY == 0) {
            printf("decode load: %u%% of real time (must stay under 100)\n",
                   (unsigned)(dec_us * 100 / play_us));
            dec_us = play_us = 0;
        }
    }

    f_close(&f);
    if (audio_on) audio_out_deinit();
    return audio_on;
}