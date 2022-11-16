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

#include "led_comm.h"

#ifdef CONSOLE_ENABLE
#include "print.h"
#endif

/* State variables */
static bool   num_lock_state    = false;
static bool   caps_lock_state   = false;
static bool   in_cmd_rec_window = false;
static bool   in_cmd_snd_window = false;

/* Set the static state variables to match the host.
 *
 * This function should be called from keyboard_post_init_user. */
void set_init_led_state(void) {
    num_lock_state  = host_keyboard_led_state().num_lock;
    caps_lock_state = host_keyboard_led_state().caps_lock;
}

/* Close out the receive window and reset the cmd_window_state
 * struct to known defaults. This function is designed to be
 * deferred a certain time after the window opens. */
uint32_t close_rcv_window(uint32_t trigger_time, void *cb_arg) {
    cmd_window_state_t *cmd_window_state = (cmd_window_state_t *)cb_arg;

#   ifdef CONSOLE_ENABLE
    uprint("CLOSE_RCV_WINDOW\n");
#   endif

    /* The conditional should not be required, but just in case... */
    if (in_cmd_rec_window) {
        in_cmd_rec_window = false;

        cmd_window_state->bit_count = 0;
        cmd_window_state->raw_led_cmd = 0;
        cmd_window_state->num_lock_count = 0;
        cmd_window_state->caps_lock_count = 0;
        cmd_window_state->valid_cmd = true;
    }

    return 0;
}

/* This is the function that tracks incoming commands and processes
 * receiving them. It calls out to a (presumably) user-defined
 * process_led_cmd function when a complete command is received.
 *
 * This function should be called from led_update_user. */
bool led_update_cmd(led_t led_state) {
    static cmd_window_state_t cmd_window_state = {
      .bit_count = 0,
      .raw_led_cmd = 0,
      .num_lock_count = 0,
      .caps_lock_count = 0,
      .valid_cmd = true
    };

    /* Only process if not currently sending a command and an appropriate state changed */
    if ((!in_cmd_snd_window) &&
        ((led_state.num_lock != num_lock_state) || (led_state.caps_lock != caps_lock_state))) {

        /* Open command window and start timer to it if we are not
         * already in the middle of one. */
        if (!in_cmd_rec_window) {
#           ifdef CONSOLE_ENABLE
                uprint("OPEN_RCV_WINDOW\n");
#           endif

            in_cmd_rec_window = true;
            defer_exec(LED_CMD_TIMEOUT, close_rcv_window, &cmd_window_state);
        }

        /* Only process the command if the current receive window
         * hasn't been invalidated. */
        if (cmd_window_state.valid_cmd) {
            /* Set num lock and caps lock bits when each is toggled
             * on and off within the receive window */
            if (led_state.num_lock != num_lock_state) {
                if ((led_state.caps_lock == caps_lock_state) &&
                    (cmd_window_state.caps_lock_count == 0) &&
                    (cmd_window_state.bit_count < LED_CMD_BITS)) {

                    cmd_window_state.num_lock_count++;

                    /* Move to the next bit if num lock was toggled twice */
                    if (cmd_window_state.num_lock_count == 2) {
                        cmd_window_state.raw_led_cmd =
                            (cmd_window_state.raw_led_cmd << 1) + NUM_LOCK_BIT;

                        cmd_window_state.num_lock_count = 0;
                        cmd_window_state.bit_count++;
                    }
                } else {
                    /* Cancel the receive window if num lock and
                     * caps lock are inter-mixed */
                    cmd_window_state.valid_cmd = false;
                }
            }

            if (led_state.caps_lock != caps_lock_state) {
                if ((led_state.num_lock == num_lock_state) &&
                    (cmd_window_state.num_lock_count == 0) &&
                    (cmd_window_state.bit_count < LED_CMD_BITS)) {

                    cmd_window_state.caps_lock_count++;

                    /* Move to the next bit if caps lock was toggled twice */
                    if (cmd_window_state.caps_lock_count == 2) {
                        cmd_window_state.raw_led_cmd =
                            (cmd_window_state.raw_led_cmd << 1) + CAPS_LOCK_BIT;

                        cmd_window_state.caps_lock_count = 0;
                        cmd_window_state.bit_count++;
                    }
                } else {
                    /* Cancel the receive window if num lock and
                     * caps lock are inter-mixed */
                    cmd_window_state.valid_cmd = false;
                }
            }
        }

        /* Process the command if complete */
        if (cmd_window_state.bit_count == LED_CMD_BITS) {
#           ifdef CONSOLE_ENABLE
                uprintf("PROCESS_LED_CMD: %d\n", cmd_window_state.raw_led_cmd);
#           endif
            process_led_cmd(cmd_window_state.raw_led_cmd);

            /* Ignore any more LED signals during this receive window */
            cmd_window_state.valid_cmd = false;
        }
    }

    /* Keep these copies of the LED states in sync with the host. */
    num_lock_state  = led_state.num_lock;
    caps_lock_state = led_state.caps_lock;
    return true;
}

/* This closes the send window, triggered after a time set by
 * the sending function. */
uint32_t close_send_window(uint32_t trigger_time, void *cb_arg) {
#   ifdef CONSOLE_ENABLE
    uprint("CLOSE_SEND_WINDOW\n");
#   endif
    in_cmd_snd_window = false;
    return 0;
}

/* This is the primary function for sending the LED command. It loops
 * with deferred execution, either pushing or releasing a lock key
 * each time it is run. It also determines how long the next loop should
 * be deferred. */
uint32_t async_send_led(uint32_t trigger_time, void *cb_arg) {
    led_cmd_out *led_cmd_ptr = (led_cmd_out *)cb_arg;
    uint32_t next_run_wait = 0;
    uint8_t  keycode = 0;

    /* Figure out which key is used and the delay on pressing it */
    switch ((led_cmd_ptr->raw_led_cmd >> led_cmd_ptr->current_bit) & 1) {
        case NUM_LOCK_BIT:
            keycode = KC_NUM;
            next_run_wait = LED_NUM_WAIT;
            break;

        case CAPS_LOCK_BIT:
            keycode = KC_CAPS;
            next_run_wait = LED_CAPS_WAIT;
            break;
    }

    /* If the key isn't pressed, just press it and keep the set delay */
    if (!led_cmd_ptr->key_down) {
        register_code(keycode);
        led_cmd_ptr->key_down = true;
    } else {
        /* Otherwise, release the key and either get the next one ready
         * or finish the sequence. */
        next_run_wait = LED_BETWEEN_WAIT;
        unregister_code(keycode);
        led_cmd_ptr->key_down = false;

        if ((led_cmd_ptr->current_bit == 0) && (led_cmd_ptr->first_stage == false)) {
            next_run_wait = 0;
            defer_exec(LED_CMD_SELF_WAIT, close_send_window, NULL);
        }

        if (led_cmd_ptr->first_stage == true) {
            led_cmd_ptr->first_stage = false;
        } else {
            led_cmd_ptr->first_stage = true;
            led_cmd_ptr->current_bit--;
        }
    }

    return(next_run_wait);
}

/* This code initiates the LED sending processing. It is designed
 * for deferred execution, looping asynchronously until no other
 * commands are in process. */
uint32_t start_led_cmd(uint32_t trigger_time, void *cb_arg) {
    static led_cmd_out static_led_cmd = {
        .current_bit = LED_CMD_BITS - 1,
        .raw_led_cmd = 0,
        .key_down = false,
        .first_stage = true
    };

    if (in_cmd_rec_window || in_cmd_snd_window) {
        /* Command already in progress, so wait for the next window */
        return(LED_CMD_TIMEOUT);
    } else {
        /* No commands in process, so start this one */
        in_cmd_snd_window = true;
        static_led_cmd.raw_led_cmd = (uintptr_t) cb_arg;
        static_led_cmd.current_bit = LED_CMD_BITS - 1;

        /* Run the first send step immediately, then defer the next one
         * the appropriate amount of time.
         */
        defer_exec(async_send_led(0, &static_led_cmd), async_send_led, &static_led_cmd);
        return(0);
    }
}

/* This function starts the LED command send process, and will
 * usually be called directly from macro code.
 *
 * If the start if deferred, the identifier for the deferred function
 * is returned. Otherwise, zero is returned. */
uint32_t send_led_cmd(uintptr_t led_cmd) {
#   ifdef CONSOLE_ENABLE
    uprintf("SEND_LED_CMD: %d\n", led_cmd);
#   endif

    /* Try to start it immediately, but start deferring it otherwise.
     * This is slightly abusing the deferred execution function by
     * passing it the LED command value directly instead of a pointer
     * to the value. This allows the code to use deferred execution
     * to handle remembering multiple future values, which means
     * there doesn't need to be an array of values. However, it also
     * means that the order that future commands are sent may not
     * be the order that they were started. */
    return(defer_exec(start_led_cmd(0, (void *) led_cmd), start_led_cmd, (void *) led_cmd));
}

/* If no commands need to be processed by a given device (commands
 * are only sent, not received), then this default handler will
 * just ignore all completed commands.
 *
 * Return value for this function is currently ignored. */
__attribute__ ((weak))
bool process_led_cmd(uintptr_t led_cmd) {
    return false;
}


/* Define a led_update_user function if no other functionality is
 * needed.
 *
 * Currently, always returns true. */
__attribute__ ((weak))
bool led_update_user(led_t led_state) {
    return(led_update_cmd(led_state));
}

/* Define a keyboard_post_init_user function if no other functionality
 * is needed. */
__attribute__ ((weak))
void keyboard_post_init_user(void) {
    set_init_led_state();
}
