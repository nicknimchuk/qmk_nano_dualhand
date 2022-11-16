/* Copyright 2022 Nick Nimchuk
 * Copyright 2022 Aidan Gauland
 * Copyright 2021 Colin Lam (Ploopy Corporation)
 * Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2019 Sunjun Kim
 * Copyright 2019 Hiroyuki Okada
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
#include QMK_KEYBOARD_H

#ifdef CONSOLE_ENABLE
#include "print.h"
#endif

/* Add headers for inter-device communication with num/caps lock */
#include "led_enum.h"
#include "features/led_comm.h"

/* Options for scrolling threshold */
#define DELTA_X_THRESHOLD 100
#define DELTA_Y_THRESHOLD 50

/* DPI options */
#define LOW_DPI     350
#define MID_DPI     750
#define HI_DPI      2000
#define PLOOPY_DPI_OPTIONS { LOW_DPI, MID_DPI, HI_DPI}
#define PLOOPY_DPI_DEFAULT 1

/* Boolean for this being the left trackball used in code */
#ifdef IS_LEFT
#define LEFT_SIDE (true)
#else
#define LEFT_SIDE (false)
#endif

/* Static variables for scrolling */
static bool   scroll_enabled = LEFT_SIDE;
static int8_t delta_x        = 0;
static int8_t delta_y        = 0;

/* Dummy keymap (no keys!) */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {{{KC_NO}}};

/* Add scrolling functionality in scrolling mode */
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (scroll_enabled) {
        delta_x += mouse_report.x;
        delta_y += mouse_report.y;

        if (delta_x > DELTA_X_THRESHOLD) {
            mouse_report.h = 1;
            delta_x        = 0;
        } else if (delta_x < -DELTA_X_THRESHOLD) {
            mouse_report.h = -1;
            delta_x        = 0;
        }

        if (delta_y > DELTA_Y_THRESHOLD) {
            mouse_report.v = -1;
            delta_y        = 0;
        } else if (delta_y < -DELTA_Y_THRESHOLD) {
            mouse_report.v = 1;
            delta_y        = 0;
        }
        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}

/* Handle communication from other QMK devices */
bool process_led_cmd(uintptr_t led_cmd) {
    switch (led_cmd) {
        case LFT_MOUSE:
            /* Set to movement if left mouse, scrolling otherwise */
#           ifdef CONSOLE_ENABLE
            if (LEFT_SIDE) {
                uprintf("LFT_MOUSE - mouse %d disabling scroll\n", !LEFT_SIDE);
            } else {
                uprintf("LFT_MOUSE - mouse %d enabling scroll\n", !LEFT_SIDE);
            }
#           endif
            pointing_device_set_cpi(MID_DPI);
            scroll_enabled = !LEFT_SIDE;
            break;

        case RGT_MOUSE:
            /* Set to movement if right mouse, scrolling otherwise */
#           ifdef CONSOLE_ENABLE
            if (!LEFT_SIDE) {
                uprintf("RGT_MOUSE - mouse %d disabling scroll\n", !LEFT_SIDE);
            } else {
                uprintf("RGT_MOUSE - mouse %d enabling scroll\n", !LEFT_SIDE);
            }
#           endif
            pointing_device_set_cpi(MID_DPI);
            scroll_enabled = LEFT_SIDE;
            break;

        case CYCLE_DPI:
            /* Cycle DPI regardless of side. Only way to
             * (temporarily) change scrolling DPI */
            cycle_dpi();
#           ifdef CONSOLE_ENABLE
            uprintf("CYC_DPI - mouse %d set to %d\n", !LEFT_SIDE, pointing_device_get_cpi());
#           endif
            break;

        case ACT_HI_DPI:
            /* Set to high DPI if this is the movement side */
            if (!scroll_enabled) {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_HI_DPI - mouse %d changing\n", !LEFT_SIDE);
#               endif
                pointing_device_set_cpi(HI_DPI);
            } else {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_HI_DPI - mouse %d ignoring\n", !LEFT_SIDE);
#               endif
            }
            break;

        case ACT_MID_DPI:
            /* Set to medium DPI if this is the movement side */
            if (!scroll_enabled) {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_MID_DPI - mouse %d changing\n", !LEFT_SIDE);
#               endif
                pointing_device_set_cpi(MID_DPI);
            } else {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_MID_DPI - mouse %d ignoring\n", !LEFT_SIDE);
#               endif
            }
            break;

        case ACT_LOW_DPI:
            /* Set to low DPI if this is the movement side */
            if (!scroll_enabled) {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_LOW_DPI - mouse %d changing\n", !LEFT_SIDE);
#               endif
                pointing_device_set_cpi(LOW_DPI);
            } else {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_LOW_DPI - mouse %d ignoring\n", !LEFT_SIDE);
#               endif
            }
            break;

        case ACT_RESET:
            /* Reset the trackball if currently in scrolling mode */
            if (scroll_enabled) {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_RESET - mouse %d resetting\n", !LEFT_SIDE);
#               endif
                reset_keyboard();
            } else {
#               ifdef CONSOLE_ENABLE
                uprintf("ACT_RESET - mouse %d ignoring\n", !LEFT_SIDE);
#               endif
            }
            break;

        default:
            /* Ignore unrecognised commands. */
#           ifdef CONSOLE_ENABLE
            uprintf("Unhandled LED command - mouse %d\n", !LEFT_SIDE);
#           endif

            return false;
    }

    return true;
}
