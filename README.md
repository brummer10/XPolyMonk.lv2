# XPolyMonk.lv2

Polyphonic version of [Xmonk.lv2](https://github.com/brummer10/Xmonk.lv2)

![xmonk](https://github.com/brummer10/XPolyMonk.lv2/raw/master/xmonk.png)


## Features

- a polyphonic (6 voices) sound generator to have some fun with.


## Midi support

- NOTE_ON/NOTE_OFF -> play note
- MODWHEEL CC 0x01 -> vowel parameter
- PITCHBEND CC 0xE0 -> pitchbend parameter
- SUSTAIN CC 0x40 -> sustain parameter
- RESET CC 0x79 -> reset all parameter
- ALL_SOUNDS_OFF/ALL_NOTES_OFF CC 0x78 0x7B -> panic


## Dependencys

- libcairo2-dev
- libx11-dev


## Build
- git submodule init
- git submodule update
- make
- make install # will install into ~/.lv2 ... AND/OR....
- sudo make install # will install into /usr/lib/lv2

that's it.
