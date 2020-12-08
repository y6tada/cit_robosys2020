/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdint>
#include <cstdio>

#define MAX_MEASURING 3000

enum {
    etime,
    evalue,
    max_log_elem
};

int main()
{
    Serial pc(USBTX, USBRX);
    pc.baud(115200);

    PwmOut ir_led(PTC10);
    ir_led.period_us(26);                       // 38.4kHz ~= IR remocon sensor modulation freq
    DigitalOut on_board_red_led(LED1);          // Red led equipped on K64F board
    DigitalOut on_board_green_led(LED2);        // Red led equipped on K64F board
    on_board_red_led.write(1);
    on_board_green_led.write(1);

    DigitalIn  remocon_sens(PTC11);             // connected to IR remocon sensor, active-low.
    DigitalIn  user_sw2(SW2);                   // user button on K64F board
    DigitalIn  user_sw3(SW3);                   // user button on K64F board
    DigitalIn  rpi_signal(PTB11);               // connected to RPI gpio out, active-high.
    Timer t;

    uint8_t  log_value[MAX_MEASURING];
    /* timer val */
    uint32_t previous_time_us               = 0;
    const uint32_t time_interval_threshold = 50;    // Sampling rate of signal emit/record.
    const uint32_t signal_emit_repeat      = 10;    // Emit same IR signal 5 times

    pc.printf("start\n");

    while (remocon_sens.read() == 1);               // wait until IR signal detected
    t.start();

    for (int i = 0; i < MAX_MEASURING; i++) {
        log_value[i] = remocon_sens.read();

        while ((t.read_us() - previous_time_us) < time_interval_threshold);
        previous_time_us = (uint32_t)(t.read_us() / time_interval_threshold) * time_interval_threshold;
    }

    for (int j = 0; j < MAX_MEASURING; j++) {
        pc.printf("%d\n", log_value[j]);
    }

    pc.printf("Ready to emit IR signal, push SW2\n");
    on_board_red_led.write(0);

    while (true) {
        ir_led = 0;
        on_board_green_led.write(1);
        while (user_sw2.read() == 1 && rpi_signal.read() == 0) {
            if (user_sw3.read() == 0) {
                pc.printf("program end by user.\n");
                return 1;
            }
        }

        on_board_green_led.write(0);
        // Emit IR Sinnal
        t.reset();
        previous_time_us = 0;
        for (int n = 0; n < signal_emit_repeat; n++) {
            for (int i = 0; i < MAX_MEASURING; i++) {
                while ((t.read_us() - previous_time_us) < time_interval_threshold) {
                    if (log_value[i] == 0) {
                        ir_led.write(0.33);
                    }
                    else {
                        ir_led.write(0);
                    }
                }
                previous_time_us = (uint32_t)(t.read_us() / time_interval_threshold) * time_interval_threshold;
            }
        }
    }

    pc.printf("program end.\n");

    return 0;
}
