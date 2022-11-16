/* Copyright 2022 Nick Nimchuk
 * Copyright 2022 Aidan Gauland
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


/* This enum defines the different commands. As noted above, if the
 * caps lock and num lock delays differ, then some commands will be
 * faster than others.
 *
 * While it's not required to define the manual binary value for
 * each command, keep in mind that no more than 2^(LED_CMD_BITS)
 * values can be used
 */

typedef enum {
    LFT_MOUSE       = 0b000,    /* Activate left mouse              */
    RGT_MOUSE       = 0b001,    /* Activate right mouse             */
    CYCLE_DPI       = 0b010,    /* Cycle DPI on all mice            */
    ACT_HI_DPI      = 0b100,    /* Set active mouse to high DPI     */
    ACT_MID_DPI     = 0b101,    /* Set active mouse to mid DPI      */
    ACT_LOW_DPI     = 0b110,    /* Set active mouse to low DPI      */
    ACT_RESET       = 0b111     /* Reset active mouse               */
} led_cmd_t;
