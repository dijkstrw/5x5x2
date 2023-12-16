Host configuration
==================

Orochi can be updated from the host using
[susanoo.py](susanoo.py). This program creates a fifo and a unix
domain socket that can be used to communicate with the keyboard.

- fifo: can be used by shell commands to push some state to the
  keyboard

- socket: can be used i.e. by socat, to communicate via terminal with
  the keyboard, even while multiple other updaters are active.

Susanoo also connects to pulseaudio to communicate volume and mute
status to the keyboard.

Desktop state (what desktop a screen is on) can be communicated using
[keys_orochi.lua](keys_orochi.lua), when using
[awesomewm](https://awesomewm.org/). This make it so that any desktop
key press on the keyboard will change all desktops for all connected
monitors. I like to use this to have a desktop for mail, a desktop for
a coding problem, and a few more for other side avenues that can pop
up while working on something. Desktop changes in awesome are
communicated back to the keyboard, and cause the keyboard two top rows
to light in the color of the active desktop.
