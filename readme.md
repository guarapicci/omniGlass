# Omni-Glass: touchpad gestures library

## (DISCLAIMER) This is W.I.P: only a small linux platform test is available right now.

## STATUS
- linux evdev platform in progress.
- built-in demo showcases slide left/right and pressure transition (as in: started pressing, stopped pressing).
- removed the old sample application.
- a misclanneous "linux nonblocking test" application demonstrates the workings and caveats of both reading in non-blocking mode and printing in small chunks.

## HOW TO RUN
You're gonna need `cmake`, GCC, a lua interpreter compatible with 5.1 and your run-of-the-mill linux distro (usually includes everything else needed).
- 1: enter the repository directory in your console with `cd`
- 2: open "templates/config.lua" on a text editor. plug into it a path to the touchpad's device file.
  - you do have a touchpad, right? is it working? did you plug it in?
  - your path will look like `/dev/input/by-id/0934:a26c touchpad` or `/dev/input/by-path/goofyahhchipset0318:platform-serio-1-event-mouse`
  - you can also choose the raw event files at `/dev/input` instead. running `evtest` as sudo will tell you which device file number relates to which device, as in: `6: touchpad-renesi`
- 3: make sure your user has permission to access the touchpad device file, otherwise the program will fail unless you have root access
- 4: run "buildsample.sh". It will build the library and run a sample that uses your touchpad configuration.

## EXPERIMENTAL CMAKE BUILDER
There is a work-in-progress cmake file for cross-platform. Here is how you use it:
- 1: run on shell: `cmake -S . -B output` to create a build folder at "output"
- 2: edit `templates/config.lua` to point to your touchpad device file.
- 3: `cd` to the output folder and run `make` to build the library.
- 4; run `cd bin; ./initialization_test`. You **must** be on the same folder as the binary, otherwise it will fail to load the libraries needed.

## What is this thing?

**summing up:** A library for touchpad gestures and a mouse/keyboard/joypad emulator that uses these gestures.

**long version:** Omni-Glass is a touchpad gesture detection engine for linux desktops. It provides two things:

- **Omni-glass engine:** An `evdev`-based gesture detector with simple API hooks for gestures (as in: "begin two-finger swipe, two-finger swipe left 10 millimiters, end finger swipe...") that can be mapped to media player volume control, file browser file picking, terminal tab selection, web browser tab switching ala-mobile-firefox, etc.
It emulates a wide variety of **non-pointer** devices out of input from your touchpad.
- **Zero-button PIE:** A programmable input emulator on top of the gesture detector that provides a fallback. The emulator allows two-step absolute tap for keyboard shortcuts, an absolute-touch cursor for mouse-heavy games (two-finger press is mouse1) and other customizable mappings from touch to keyboard, joypad, mouse, D-bus, socket IPC and more.

## We are not touchégg.
The purpose of Omni-Glass is to implement arbitrary gestures for each application, instead of just doing desktop-wide actions like switch window & expand desktop.
We do provide a PIE, mainly for videogames, but applications with source code available can (and should) properly map touch gestures to actions due to how contextual any given gesture could be.´
