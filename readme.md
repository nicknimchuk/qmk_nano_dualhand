# qmk_nano_dualhand: a keymap designed for two Ploopy Nano trackballs
This keymap configures two Ploopy Nano trackballs, one for each hand. At any given time, one will be for mouse movement, while the other is used for scrolling. This can be swapped dynamically using commands from a QMK keyboard, potentially reducing injury or strain by not restricting mouse movement to a single hand. 

![Pictured: two Ploopy Nano trackballs with a ZSA Moonlander keyboard on the Platform accessory](https://imgur.com/hrQaC6u.jpg)

## Inter-QMK communication
The primary feature this code provides is a num/caps lock communication feature based on the [lkbm](../lkbm) Nano keymap, which was itself based on [maddie](../maddie). It provides code for sending and receiving commands between QMK devices by toggling the caps lock and num lock keys for "bits" of the commands. The main advantages over the previous versions of this feature are:

* The feature is mostly broken out into separate files, so it's easier to apply to other devices and keymaps.
* The command length is an arbitrary (pre-defined) number of bits, so any number of commands can be created at the cost of slower commands.
* There are many timings that can be easily tweaked to try to find the right balance of speed and command reliability. These timings do not have to influence any other settings in QMK.
* The commands are sent asynchronously, so even if a command takes a full second to send, you can use the device while it is sent.

There's also some disadvantages:

* Commands will likely take longer to allow for two-way communication if desired.
* Bits are only processed if the given lock is toggled twice without the other lock toggling. This reduces misinterpreted commands, but also means that some commands that [lkbm](../lkbm) would handle due to interleaved lock commands will be ignored.
* The asynchronous nature means that typing could happen while caps lock or num lock is temporarily enabled (or temporarily disabled). This can be slightly mitigated by the fact that one command will only use caps lock, and another command will only use num lock (but the rest will use both).
* This feature is not written to minimize flash space. This is not a problem with the Nano, which has plenty of free space by default, but may cause problems with some keyboards. The older options do not require any additional code space to send commands.

For the dual-handed Nano setup, the commands allow swapping of mouse movement and scrolling between hands depending on which side of the keyboard is used for mouse clicks. The feature could also be used to send command to the keyboard when the trackball is moved and activate a mouse layer and allow the keyboard to send a command back to the trackball when the layer has been deactivated (so the trackball knows to send the command again on the next movement).

The pre-implemented commands help handle two trackballs by only applying most DPI changes to the movement ball, not the one currently scrolling. There's also a reset command that is only applied to the scrolling side, so the movement side is still usable while reprogramming (and the Nano does not need to be taken apart or even unplugged).

## Setting up inter-QMK communication
There are several files used for implementing and configuring the communication feature. This allows the definitions to happen in one place, and then be used across multiple devices. This would likely better be implemented as a user-space feature, but that is just a potential future revision. The following shows the basic changes needed across files to add it to a specific keymap and configure the feature.

Start by putting this repository in the QMK source file structure, in the folder `keyboards/ploopyco/trackball_nano/keymaps/dualhand` (or other name for the final folder if desired).

### `led_config.h` in the `dualhand` folder
Change the settings by adding `#define` statements as desired. Most settings are related to the speed of the lock keys. If no `#define` statement is used, the default is a minimal (fastest) value that may not work on any real system. The settings in this repository are much more conservative, which was needed on the system that it was developed to work under load and with the [barrier](https://github.com/debauchee/barrier) software KVM running (on Windows).

### `led_enum.h` in the `dualhand` folder
This file contains the communication command definitions. Change as desired. By default, there are 3 bits in a command with a limit of 8 unique commands, but that can be decreased or increased as needed. Note that the command names are not custom keycodes and should not be added to a keymap, and they must not have names that are the same as a keycode.

### `config.h` in the keyboard keymap folder
`#include` the `led_config.h` file using a relative link. It will likely look similar to the below, possibly with a different number of `../`s.

```c
#include "../../../ploopyco/trackball_nano/keymaps/dualhand/led_config.h"
```

### `rules.mk` in the keyboard keymap folder
Add a relative `SRC` link to the `features/led_comm.c` source file in the `dualhand` folder. `DEFERRED_EXEC_ENABLE` must also be enabled. It will look similar to this:

```
SRC += ../../../ploopyco/trackball_nano/keymaps/dualhand/features/led_comm.c
DEFERRED_EXEC_ENABLE = yes
```

### `keymap.c` in the keyboard keymap folder

Near the top of the file, add relative `#include`s for `led_enum.h` and `features/led_comm.h` in the `dualhand` folder. It will look like:

```c
#include "../../../ploopyco/trackball_nano/keymaps/dualhand/led_enum.h"
#include "../../../ploopyco/trackball_nano/keymaps/dualhand/features/led_comm.h"
```

If the keymap has a custom `led_update_user(led_t led_state)` function implemented, then add the following line to it:
```c
return(led_update_cmd(led_state));
```

If the keymap has a custom `keyboard_post_init_user(void)` function implemented, then add the following line to it:
```c
set_init_led_state();
```

If the keymap will be used to process incoming commands, then add a `process_led_cmd` function definition. The following is an example to start with.

```c
bool process_led_cmd(uintptr_t led_cmd) {
    switch (led_cmd) {
        case EXAMPLE:
            /* An example incoming command to be processed */
#           ifdef CONSOLE_ENABLE
            uprint("EXAMPLE - exampling");
#           endif
            example_work();
            break;

        default:
            /* Ignore unrecognised commands. */
#           ifdef CONSOLE_ENABLE
            uprint("Unhandled LED command\n");
#           endif

            return false;
    }

    return true;
}
```

If the keymap is sending commands, add macros to do so. The following section has examples.

#### Sample keyboard macros
The following code shows some example macros to send commands from a keyboard's `keymap.c` in the `process_record_user` function, likely within a `switch` statement. The keys would need to be included in the custom keycodes and keymap as well.
```c
/* A basic command sent on keypress */
case RST_MOU:
    if (record->event.pressed) {
        send_led_cmd(ACT_RESET);
        return false;
    }
    break;
    
/* A key that sets the movement side to high DPI while pressed and
 * returns to medium DPI when released */
case TMP_HDPI:
    if (record->event.pressed) {
        send_led_cmd(ACT_HI_DPI);
    } else {
        send_led_cmd(ACT_MID_DPI);
    }
    return false;
    
/* This is a layer shift key that sets the active trackball to the
 * right if needed. It requires that is_mouse_right is declared
 * previously:
static bool is_mouse_right = true;
 * It also requires that the key was already defined as a layer
 * switch, such as for the _MBUTTON layer:
#define MB_LEFT  MO(_MBUTTON)
 */
case MB_LPRN:
    if (record->event.pressed && (!is_mouse_right)) {
        is_mouse_right = true;
        send_led_cmd(RGT_MOUSE);
    }
    /* Return true to maintain layer functionality */
    return true;
    
/* This macro changes the active mouse to the left side (using the
 * is_mouse_right boolean shown previously) if the right side is
 * currently active. Otherwise, it acts as a left mouse button. */
case B1_LEFT:
    if (record->event.pressed) {
        if (is_mouse_right) {
            is_mouse_right = false;
            send_led_cmd(LFT_MOUSE);
        } else {
            register_code16(KC_BTN1);
        }
    } else {
        unregister_code16(KC_BTN1);
    }
    return false;
```

## Flashing the two Ploopy Nano trackballs
Each Nano must be assigned to either the left or right side by the firmware. To do that, put to the right side ONLY into flashing mode, and then use this build/flash command:
```
qmk flash --keyboard ploopyco/trackball_nano --keymap dualhand -e MOUSE_SIDE=right
```

Then, put the left side ONLY into flashing mode, and use this command
```
qmk flash --keyboard ploopyco/trackball_nano --keymap dualhand -e MOUSE_SIDE=left
```

By default, when started, the left trackball will be in scroll mode, and the right will be in movement mode.
