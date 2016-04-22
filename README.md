# Atreus Firmware

This is the firmware for the [Atreus keyboard](https://github.com/technomancy/atreus).

This is my ([jturner](https://github.com/jturner)) personal fork of the
codebase.

This branch is specific to the Atreus variant that uses the
[A-Star Micro](http://www.pololu.com/product/3101).

## Layout

Only a handful of punctuation marks (and no digits) are available
unshifted, and all the modifiers are on the bottom row:

     q     w     e     r     t       ||       y     u     i     o    p
     a     s     d     f     g       ||       h     j     k     l    ;
     z     x     c     v     b       ||       n     m     ,     .    /
    esc   tab  super shift bksp ctrl || alt space  fn     -     '  enter

The numbers and most of the punctuation are on the fn layer with a
numpad-style arrangement under the right hand:

     !     @     {     }     |       ||     pgup    7     8     9    *
     #     $     (     )     `       ||     pgdn    4     5     6    +
     %     ^     [     ]     ~       ||       &     1     2     3    \
    L2  insert super shift bksp ctrl || alt space   fn    .     0    =

The `L2` key switches it to the function layer, and tapping `L0` here
brings it back to the first layer.

    insert home up   end   pgup      ||      up     F7    F8    F9   F10
     del   left down right pgdn      ||     down    F4    F5    F6   F11
                           reset     ||             F1    F2    F3   F12
               super shift bksp ctrl || alt space   L0

If you want easier access to the arrow keys, you can try the
`qwerty_alt` layout, which puts them on the fn layer:

     !    @     up     {    }        ||     pgup    7     8     9    *
     #  left   down  right  $        ||     pgdn    4     5     6    +
     [    ]      (     )    &        ||       `     1     2     3    \
    L2  insert super shift bksp ctrl || alt space   fn    .     0    =

Further alternate layouts are included; see "Customizing Layout" below
for making your own.

* colemak
* softdvorak (assumes the OS is set to dvorak, and shuffles punctuation accordingly)
* hardwaredvorak (lets you type in dvorak even if the OS is set to qwerty)
* multidvorak (combines softdvorak and hardwaredvorak)

## Usage

Install
[gcc-avr](http://www.nongnu.org/avr-libc/user-manual/install\_tools.html)
and [avrdude](http://www.nongnu.org/avrdude/).

On OpenBSD:

    $ doas pkg_add avrdude gcc-avr avr-libc

Activate the bootload with reset. Run `doas make upload`.

To use another C layout, copy it to `layout.h`; for example `cp
multidvorak.h layout.h`.

## Pinout

This is the pinout for the PCB-based Atreus using an A-Star
microcontroller. ([Mark 3](https://github.com/technomancy/atreus/blob/master/changelog.md)
onwards.)

Outputs:

    |------------+----+----+----+----|
    | row number |  0 |  1 |  2 |  3 |
    |------------+----+----+----+----|
    | pin number | D0 | D1 | D3 | D2 |
    |------------+----+----+----+----|

Inputs:

    |---------------+----+----+----+----+----+----+----+----+----+----+----|
    | column number |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 |
    |---------------+----+----+----+----+----+----+----+----+----+----+----|
    | pin number    | B7 | B6 | F7 | F6 | B6 | D4 | E6 | B4 | B5 | C6 | D7 |
    |---------------+----+----+----+----+----+----+----+----+----+----+----|

If you soldered the PCB in upside down, never fear! This can be fixed
in the firmware without removing the switches and resoldering. Simply
run `make SWAPCOLUMNS=yes USB=...` to use a reversed pinout
configuration. You may need to run `make clean` before using this option.

## Customizing Layout

Layouts in C are arrays of integers. For basic key presses, use the
keycodes defined in `usb_keyboard.h`. For modified key presses use the
`CTRL()`, `SHIFT()`, `ALT()` and `GUI()` macros from `atreus.c`. These
may be stacked for holding down multiple modifiers together. Layouts
can also include references to functions to execute. Place a `void`
function pointer in the `layout_functions` array and use the
`FUNCTION()` macro providing the index of the function, and that
keypress will cause the function to be invoked.

The `layouts` pointer should be set to an array of layouts. Every scan
through the keyboard matrix will set the current layout to the
`current_layout_number`th element of the `layouts` array. To make a
keybinding that changes the current layout use the `LAYER()` macro.

However, most functions will be called the final pass where each
keypress is looked up in the currently active layout. That means if
you have a function that changes the current layout, it needs to run
sooner so it can affect regular keycode lookups. The `PRE_FUNCTION()`
macro will trigger functions that run on a separate pre-invoke pass
before the rest of the keycodes are looked up, so this is how
layer-changing functions should be defined. There is also a
`per_cycle` function you can define which will run once per completed
scan.

## How it works

Since the microcontroller has a limited number of pins, the switches
are wired in a matrix where each has its positive contact connected to
those in the same row and its negative contact wired in with those
above and below it. Reading the state of the switches can only happen
a row at a time: a single row (output) pin is brought low, and all the
column inputs are read. Any of them that read low are recorded as a
keypress. Low voltage is used to for pressed keys because each input
pin has a built-in pullup resistor.

However, because of the electrical properties of switches, it's
necessary to go through a
[debouncing](https://en.wikipedia.org/wiki/Switch#Contact_bounce)
process as the switches settle. This means taking a few scans over the
matrix and waiting until you get N successive reads of the same state
before counting any single keypress or release as legitimate.

## License

Copyright © 2014-2016 Phil Hagelberg and contributors

Released under the [GNU GPL version 3](https://www.gnu.org/licenses/gpl.html).
