# Omni-Glass: touchpad gestures library

## (DISCLAIMER) This is W.I.P: no code yet.

## What is this thing?

**summing up:** A library for touchpad gestures and a mouse/keyboard/joypad emulator that uses these gestures.

**long version:** Omni-Glass is a touchpad gesture detection engine for linux desktops. It provides two things:

- **Omni-glass engine:** An `evdev`-based gesture detector with simple API hooks for gestures (as in: "begin two-finger swipe, two-finger swipe left 10 millimiters, end finger swipe...") that can be mapped to media player volume control, file browser file picking, terminal tab selection, web browser tab switching ala-mobile-firefox, etc.
It emulates a wide variety of **non-pointer** devices out of input from your touchpad.
- **Zero-button PIE:** A programmable input emulator on top of the gesture detector that provides a fallback. The emulator allows two-step absolute tap for keyboard shortcuts, an absolute-touch cursor for mouse-heavy games (two-finger press is mouse1) and other customizable mappings from touch to keyboard, joypad, mouse, D-bus, socket IPC and more.

## We are not touchégg.
The purpose of Omni-Glass is to implement arbitrary gestures for each application, instead of just doing desktop-wide actions like switch window & expand desktop.
We do provide a PIE, mainly for videogames, but applications with source code available can (and should) properly map touch gestures to actions due to how contextual any given gesture could be.´
