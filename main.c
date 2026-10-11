#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include "config.h"
#include "storage.h"
#include "mp3_player.h"

int main(void) {
#if OVERCLOCK_KHZ > 125000
    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(10);
    set_sys_clock_khz(OVERCLOCK_KHZ, true);
#endif

    stdio_init_all();

    // Give USB serial up to 5 s to connect so the first messages aren't lost.
    absolute_time_t until = make_timeout_time_ms(5000);
    while (!stdio_usb_connected() && !time_reached(until)) sleep_ms(50);

    if (!storage_mount()) {
        while (true) sleep_ms(1000);
    }
    storage_list_root();

    puts("Playing test.mp3");
    mp3_play_file("0:/test.mp3");
    puts("Done");

    while (true) sleep_ms(1000);
}