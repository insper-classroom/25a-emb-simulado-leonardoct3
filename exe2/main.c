/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

volatile bool fired_1 = false;
volatile bool fired_2 = false;

volatile int echo_flag_1 = 0;
volatile int echo_flag_2 = 0;

volatile int time_end_1 = 0;
volatile int time_end_2 = 0;

volatile int time_start_1 = 0;
volatile int time_start_2 = 0;

const int ECHO_PIN_1 = 12;
const int TRIGGER_PIN_1 = 13;

const int ECHO_PIN_2 = 18;
const int TRIGGER_PIN_2 = 19;




bool timer_1_callback(repeating_timer_t *rt) {
    fired_1 = true;
    return false;
}

bool timer_2_callback(repeating_timer_t *rt) {
    fired_2 = true;
    return false;
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == ECHO_PIN_1) {
        if (events & GPIO_IRQ_EDGE_RISE) {
            time_start_1 = to_us_since_boot(get_absolute_time());
            
        } if (events & GPIO_IRQ_EDGE_FALL) {
            time_end_1 = to_us_since_boot(get_absolute_time());
            
            echo_flag_1 = 1;
        }
    }
    if (gpio == ECHO_PIN_2) {
        if (events & GPIO_IRQ_EDGE_RISE) {
            time_start_2 = to_us_since_boot(get_absolute_time());
        } if (events & GPIO_IRQ_EDGE_FALL) {
            time_end_2 = to_us_since_boot(get_absolute_time());
            echo_flag_2 = 1;
        }
    }
}

void trigger_sensor_1(int *flag) {
    gpio_put(TRIGGER_PIN_1, 1);
    sleep_us(10);
    gpio_put(TRIGGER_PIN_1, 0);
    *flag = 1;
}

void trigger_sensor_2(int *flag) {
    gpio_put(TRIGGER_PIN_2, 1);
    sleep_us(10);
    gpio_put(TRIGGER_PIN_2, 0);
    *flag = 1;
}


int main() {
    stdio_init_all();

    gpio_init(ECHO_PIN_1);
    gpio_set_dir(ECHO_PIN_1, GPIO_IN);
    gpio_pull_down(ECHO_PIN_1);
    gpio_set_irq_enabled_with_callback(
        ECHO_PIN_1,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &gpio_callback
    );

    gpio_init(TRIGGER_PIN_1);
    gpio_set_dir(TRIGGER_PIN_1, GPIO_OUT);

    gpio_init(ECHO_PIN_2);
    gpio_set_dir(ECHO_PIN_2, GPIO_IN);
    gpio_pull_down(ECHO_PIN_2);
    gpio_set_irq_enabled(
        ECHO_PIN_2,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true
    );

    gpio_init(TRIGGER_PIN_2);
    gpio_set_dir(TRIGGER_PIN_2, GPIO_OUT);

    repeating_timer_t timer_1, timer_2;

    int trigged_1 = 0;
    int trigged_2 = 0;

    add_repeating_timer_ms(1000, timer_1_callback, NULL, &timer_1);
    add_repeating_timer_ms(1000, timer_2_callback, NULL, &timer_2);

    trigger_sensor_1(&trigged_1);
    trigger_sensor_2(&trigged_2);
    
    while (true) {

        if (fired_1) {
            fired_1 = false;

            if (trigged_1) {
                trigged_1 = 0;

                if (!echo_flag_1) {
                    printf("Erro no sensor 1!\n");
                    trigger_sensor_1(&trigged_1);
                    add_repeating_timer_ms(500, timer_1_callback, NULL, &timer_1);
                    continue;
                }
                
                
                int dt_1 = time_end_1 - time_start_1;
                
                int distancia_1 = (int) ((dt_1 * 0.0343) / 2.0);

                printf("Sensor 1 - dist: %d cm\n", distancia_1);
                echo_flag_1 = 0;
                trigger_sensor_1(&trigged_1);
                add_repeating_timer_ms(500, timer_1_callback, NULL, &timer_1);
            }
        }

        if (fired_2) {
            fired_2 = false;

            if (trigged_2) {
                trigged_2 = 0;

                if (!echo_flag_2) {
                    printf("Erro no sensor 2!\n");
                    trigger_sensor_2(&trigged_2);
                    add_repeating_timer_ms(500, timer_2_callback, NULL, &timer_2);
                    continue;
                }

                int dt_2 = time_end_2 - time_start_2;
                
                int distancia_2 = (int) ((dt_2 * 0.0343) / 2.0);

                printf("Sensor 2 - dist: %d cm\n", distancia_2);
                echo_flag_2 = 0;
                trigger_sensor_2(&trigged_2);
                add_repeating_timer_ms(500, timer_2_callback, NULL, &timer_2);

            }
        }

    }

    return 0;
}
