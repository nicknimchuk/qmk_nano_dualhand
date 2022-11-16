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


/* These are the various tuning options for the LED communication feature.
 * Defaults are shown in the comments. They only need to be defined if a
 * different value is desired.
 */

/* Define whether caps lock sends ones or zeroes. If the delay differs
 * between caps lock and num lock, choosing this value determines which
 * commands are faster.
#define CAPS_LOCK_BIT 1
 */

/* Define how long the num lock key is held. At least 1 ms is needed for
 * the code to work, and the default tap code delay is 0, so it only uses
 * that value if it has been set to higher than 0.
 *
 * Longer values may be needed to get the OS to toggle the
 * LED when they are sent in rapid succession.
 *
 * Note that the delay needed can depend on many factors, such as current
 * system load. That means that shorter values may work at times, but may
 * need to be increased if it doesn't work consistently.
#define LED_NUM_WAIT (TAP_CODE_DELAY ? TAP_CODE_DELAY : 1)
 */
#define LED_NUM_WAIT 60

/* Similarly define how long the caps lock key is held. By default, QMK uses
 * a longer value for caps lock due to MacOS ignoring shorter values.
#define LED_CAPS_WAIT (TAP_HOLD_CAPS_DELAY ? TAP_HOLD_CAPS_DELAY : 1)
 */
#define LED_CAPS_WAIT 60

/* Define the time between key presses. This cannot be zero due to how it
 * is used in the code.
#define LED_BETWEEN_WAIT 1
 */
#define LED_BETWEEN_WAIT 5


/* Define how many bits the messaging system will use. Higher values will
 * allow more unique messages to be defines, but higher values will also
 * require more time to be sent and received.
#define LED_CMD_BITS 3
 */
#define LED_CMD_BITS 3


/* Define how long QMK waits for the command to finish after receiving the
 * first LED change. Any extra LED changes after a command has been
 * completed will be ignored until this time has passed.
#define LED_CMD_TIMEOUT (2 * (LED_CAPS_WAIT + LED_BETWEEN_WAIT) * LED_CMD_BITS + 100)
 */
#define LED_CMD_TIMEOUT 1200

/* When a given device is sending a command, it ignores any LED changes.
 * This value determines how long QMK continues to ignore LED changes
 * after finishing sending a command. Additional commands will also wait
 * until this window closes before being sent.
#define LED_CMD_SELF_WAIT (LED_CMD_TIMEOUT - (2 * (LED_CAPS_WAIT + LED_BETWEEN_WAIT) * LED_CMD_BITS) + 100)
 */
#define LED_CMD_SELF_WAIT 1000
