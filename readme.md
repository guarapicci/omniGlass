# omniGlass: touchpad gestures library

## (DISCLAIMER) This is W.I.P; examples available at src/tests/





## STATUS (v0.2.1)
- Applications can now request the coordinates of contact points directly to perform their own gesture detection (exported via `omniglass_get_raw_report()`)
- Touchpads are now autodetected if the specific device chosen on the config file was not found.
- Available on the AUR.

https://github.com/guarapicci/mpv-omniGlass/assets/143821200/d76cfa4c-902c-4cea-83ea-c51126f85f82

https://github.com/guarapicci/omniGlass/assets/143821200/0ef46864-e9b8-4218-96c3-8b4d17b1e7ef

[mpv-omniglass](https://github.com/guarapicci/mpv-omniGlass/tree/main) is the first piece of software to link to libomniglass.

### licensing
as of this writing, this software is licensed under GPLv3.

## Get OmniGlass (for arch linux users)
- 1: install from the AUR [as instructed on their wiki](https://wiki.archlinux.org/title/Arch_User_Repository#Installing_and_upgrading_packages).
- 2: make your user has read-access to the touchpad's device file.
- 2: that's it. Your computer now has OmniGlass!

## Build from source
- 1: go to project folder, open a shell.
- 2: open `templates/config.lua` on a text editor, add your touchpad (path: "/dev/input/by-id/somethingsomething") there (**make sure your user has read access to the touchpad file!**).
- 3: with superuser permissions, run the command line: `source utils.sh && cmgen && rebuild && reinstall && debug edge_swipe_multi`.
- 4: That's it. You now have omniGlass installed and descriptive text will appear on the shell if you drag your finger accross the borders of the touchpad.
  
## Detailed instructions (if quick start failed)
You're gonna need `cmake`, GCC, a lua interpreter compatible with 5.1 and your run-of-the-mill linux distro (usually includes everything else needed).

- 1: open "templates/config.lua" on a text editor. plug into it a path to the touchpad's device file.
  - you do have a touchpad, right? is it working? did you plug it in?
  - your path will look like `/dev/input/by-id/0934:a26c touchpad` or `/dev/input/by-path/goofyahhchipset0318:platform-serio-1-event-mouse`
  - you can also choose the raw event files at `/dev/input` instead. running `evtest` as sudo will tell you which device file number relates to which device, as in: `6: touchpad-renesi`. **beware: files on this folder switch numbers often.**
- 2: make sure your user has permission to access the touchpad device file, otherwise the program will fail unless you have root access
- 3: Inside the project's root folder, call `source utils.sh` to load the functions into your unix shell, then:
  - use `cmgen` to run cmake with the project's defaults
  - use `rebuild` to recompile sources... presuming it's a makefile.
  - use `debug <binary>` to run an executable. `<binary>` must be replaced by one of:
    - `edge_swipe` - an example that detects motion at the lower 10% of the touchpad by default (edge width can be changed in coonfig file)
    - `swipetap` - an example that detects left/right swipel
    - `init` - this executable just runs platform init. it fails if the system cannot connect to your touchpad.
  - use `reinstall` to install omniGlass as a shared library. This enables omniGlass gestures on applications that support it.
## example: swipetap.c (shows how to use the API)
```C
#include "../omniglass.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void on_slide(double value) {
    if (value < 0)
        printf("moved left\t\t%g\n", value);
    else if (value > 0)
        printf("moved right\t\t%g\n",value);
    fflush(stdout);
}

int main(int argc, char **argv) {
        
    struct omniglass *handle;
    if (omniglass_init(&handle) != OMNIGLASS_RESULT_SUCCESS)
        fprintf(stderr,"could not initialize omniglass.\n");
    omniglass_listen_gesture_slide(handle,&on_slide);
    fflush(stdout);
    printf("starting event loop\n");
    while (true) {
        omniglass_step(handle);
    }
    
    return 0;
}

```

## WINDOWS PORTING
A windows port is not available (and there is no demand for it) at the moment.

To those willing to start a windows port of omniglass, i suggest you start from `platform_linux.c`, replacing all the evdev stuff with [libhid](https://github.com/bfoz/libhid) equivalents.

## What is this thing?

**summing up:** A library for touchpad gestures and a mouse/keyboard/joypad emulator that uses these gestures.

**long version:** Omni-Glass is a touchpad gesture detection engine for linux desktops. It provides two things:

- **Omni-glass engine:** An `evdev`-based gesture detector with simple API hooks for gestures (as in: "begin two-finger swipe, two-finger swipe left 10 millimiters, end finger swipe...") that can be mapped to media player volume control, file browser file picking, terminal tab selection, web browser tab switching ala-mobile-firefox, etc.
It emulates a wide variety of **non-pointer** devices out of input from your touchpad.
- **Zero-button PIE:** A programmable input emulator on top of the gesture detector that provides a fallback. The emulator allows two-step absolute tap for keyboard shortcuts, an absolute-touch cursor for mouse-heavy games (two-finger press is mouse1) and other customizable mappings from touch to keyboard, joypad, mouse, D-bus, socket IPC and more.

## We are not touchégg.
The purpose of Omni-Glass is to implement arbitrary gestures for each application, instead of just doing desktop-wide actions like switch window & expand desktop.
We do plan on providing a PIE, mainly for videogames, but applications with source code available can (and should) properly map touch gestures to actions due to how contextual any given gesture could be.
