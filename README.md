5x5x2 aka Orochi!
=================

This is my second custom keyboard, a 5x5 cherry mx matrix based on the STM32F1
with libopencm3. The project contains firmware, schematics and a board layout
in kicad.

![Board Front Picture](schematic/pictures/front.jpg)

This project is intended as a platform for custom keyboard development. It
supports:
- usb keyboard scancodes
- bios boot and nkro
- system / consumer codes
- generation of normal mouse events
- usb serial interface
- an automouse mode for fast-clicking
- programmable macro keys via serial
- storing current configuration in "userflash"
- sk68xx backlight leds
- rotary encoder

The board is programmed using a 4-pin SWIO/SWCLK .1'' header.

Default firmware / my usecase
-----------------------------

This is my macropad; first two rows select my current workspace in the
windowmanager, and the 10 keys light together in that workspace color
as a visual reminder where I am at.

The rotary encoder is for volume control. Rotary press = mute/unmute.

Third row of keys are media keys.

Building
========

    git submodule init
    git submodule update
    cd docker
    docker build -t stm32-gcc .
    cd ..
    ./docker/dev.sh
    make -C libopencm3
    make

Features
========

Keyboards
---------

The board provides 3 types of keyboards via different usb
endpoints.

1. The first is the usb standard bios keyboard definition, which
   allows the keyboard also to emit key codes to hosts with limited
   usb hid support. (This used to be the case in <2015 style bioses,
   hence the name.)

2. The second keyboard is a n-key-rollover, which exposes a keyboard
   that allows 224 usb keyboard scancodes to be pressed
   simultaneously. Also, because we do not need to adhere to the bios
   boot standard, this endpoint's interval can set much lower = it
   types much faster.

3. The third keyboard can emit so called consumer and system
   codes. Example consumer key codes are PLAY, PAUSE, but also MARK,
   CLEARMARK, REPEATFROMMARK. So everything the typical multimedia
   keys emit and then some.

Mouse
-----

Operate the rodent from your keyboard! There is support for x, y, 5
buttons and a vertical and horizontal scrollwheel action.

Serial
------

The board exposes a serial port for logging and
configuration. Configuration is done using commands, with these
general rules:

    - Commands with lowercase letters request configuration
      information

    - Commands with capital letters set configuration

    - Arguments often are two hexadecimal digits long; e.g. 00, 12,
      fa. If more digits are required, this is denoted in the argument
      specification by a colon, like so <argument:4>.

The available commands are:

    ?  - show a terse description of available commands.

    i  - show usb info strings; contains the git-describe tag of the
         current firmware, so mission critical to some, useless to
         everybody else.

    de - dump the rgb ease functions per key
    dg - dump the rgb group per key
    dk - dump the keymap
    dp - dump the palette
    dr - dump the rotary configuration

    B  - set the bottom set of 8 leds (backlight) to custom rgb values.
         Takes a RGB argument of the form <rgb:6> times 8 for all leds.

    C  - set the top set of 25 leds (frontlight) to custom rgb values.
         Takes a RGB argument of the form <rgb:6> times 25 for all leds.

    E  - define an rgbease function for a key; takes arguments <pressed>
         <row><column><color><function><step><round><group>.

    G  - assign a particular key to a RGB group. Takes the arguments
         <row><column><group>.

    K  - redefine a key in the keymap, takes argument of the form
         <layer><row><column><type><arg1><arg2><arg3>.

    A  - clear all macro keys.

    M  - define one macro key, takes an argument of the form
         <number><oftenusedstring>. The number is a two hexdigits, the
         string can be upto 32 7-bit ascii chars long and is terminated
         with a newline.

    N  - set keyboard mode to <argument>; 00 for bios and 01 for nkro.
         Default is bios.

    P  - set palette color, takes arguments
         <number><hue:4><saturation><value>

    R - redefine the rotary command, takes a argument of
        the form <layer><direction><type><arg1><arg2><arg3>.

    L - load configuration from flash

    S - save configuration to flash

    Z - clear the configration flash, revert to "factory" keymap at
        next powerup.

Command interpretation starts after receiving a newline.

Color easing
------------
Key down (and up) events can be tied to an rgb easing function. This
means that a keypress will result in a led slowly changing to a
particular color.

The easing functions defined are:

    - COLOR_FLASH : ease up towards a color in <step> increments, and
    then fade away.

    - COLOR_HOLD  : ease towards a color in <step> increments and hold.

    - DIM         : ease away in <step>s from color, end in off.

    - BRIGHTEN    : ease towards color in <step>s, end in off.

    - RAINBOW     : use on <round> to walk all hues, set <round> to
    number of times to repeat.

    - BACKLIGHT   : use backlight color at backlight intensity.

An easing definition includes step, round and group. Step is used to
count where we are in the easing, or in what pace the ease is
run. Round is used when an easing has multiple rounds (flash = towards
color (0), towards black (1), off (2)). Group, finally, defines to
what group this easing will be applied. This way multiple keys can
light together if one key is pressed.

Palette
-------

Colors in easings are assiged using a color index in the
palette. The default palette strives for a selection of colors that
are visually distinct from another.

The rotary action forward, backward and backlight colors have fixed
indexes, and can be set by adjusting their assigned palette colors.

Automouse
---------

Automouse is a mouse clicker, with optional speed setting and wiggle
amounts. This allows you to click real fast, and wiggle the mouse
while using your hands for something else.

Macros
------

Macros set via serial are lost at power off unless the configuration
is written to flash. Loading from flash happens automatically at
powerup.

The strings that you provide via serial need to be translated into usb
keycodes, so currently only 7-bit ascii strings are supported.

Setting macros via the shell is easy:

    echo -e "\nM01Nevergonnagiveyouup!\n" > /dev/ttyACM0

Notes:

- The initial newline is a trick to make sure our M will be considered
  the start of a command, no matter what has been put to the serial
  before by you, some driver or the os.
- This particular example defines the macro for macro key number 1.
- You need to have a macro key 1 in your keymap, otherwise you have
  nothing to trigger the macro.
- There is intentionally no way to display stored macros.
