/* Copyright 2022 Nick Nimchuk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include QMK_KEYBOARD_H


/* Define the defaults for any configuration that isn't overriden */

#ifndef CAPS_LOCK_BIT
#define CAPS_LOCK_BIT 1
#endif

#define NUM_LOCK_BIT (1 - CAPS_LOCK_BIT)

#ifndef LED_NUM_WAIT
#define LED_NUM_WAIT (TAP_CODE_DELAY ? TAP_CODE_DELAY : 1)
#endif

#ifndef LED_CAPS_WAIT
#define LED_CAPS_WAIT (TAP_HOLD_CAPS_DELAY ? TAP_HOLD_CAPS_DELAY : 1)
#endif

#ifndef LED_BETWEEN_WAIT
#define LED_BETWEEN_WAIT 1
#endif

#ifndef LED_CMD_BITS
#define LED_CMD_BITS 3
#endif

#ifndef LED_CMD_TIMEOUT
#define LED_CMD_TIMEOUT (2 * LED_CAPS_WAIT * LED_CMD_BITS + 100)
#endif

#ifndef LED_CMD_SELF_WAIT
#define LED_CMD_SELF_WAIT (LED_CMD_TIMEOUT - (2 * (LED_CAPS_WAIT + LED_BETWEEN_WAIT) * LED_CMD_BITS) + 100)
#endif


void set_init_led_state(void);

bool process_led_cmd(uintptr_t led_cmd);

bool led_update_cmd(led_t led_state);

uint32_t send_led_cmd(uintptr_t led_cmd);


typedef struct {
    uint8_t   bit_count;
    uintptr_t raw_led_cmd;
    uint8_t   num_lock_count;
    uint8_t   caps_lock_count;
    bool      valid_cmd;
} cmd_window_state_t;

typedef struct {
    uint8_t   current_bit;
    uintptr_t  raw_led_cmd;
    bool      key_down;
    bool      first_stage;
} led_cmd_out;
