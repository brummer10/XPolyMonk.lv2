# XPolyMonk.lv2

Polyphonic version of [Xmonk.lv2](https://github.com/brummer10/Xmonk.lv2)

![xmonk](https://github.com/brummer10/XPolyMonk.lv2/raw/master/xmonk.png)

## Features

- a polyphonic (12 voices) sound generator.


## Midi support

- NOTE_ON/NOTE_OFF (VELOCITY) -> play note
- MODWHEEL CC 0x01 -> vowel parameter
- PITCHBEND CC 0xE0 -> pitchbend parameter
- ATTACK CC 0x49 -> attack parameter
- SUSTAIN CC 0x40 -> sustain parameter
- RELEASE CC 0x48 -> release parameter
- RESET CC 0x79 -> reset all parameter
- VOLUME CC 0x27 -> gain parameter
- DETUNE CC 0x5E -> Celeste Level or Detune
- ALL_SOUNDS_OFF/ALL_NOTES_OFF CC 0x78 0x7B -> panic


## Dependencys

- libcairo2-dev
- libx11-dev
- lv2-dev


## Build
- git submodule init
- git submodule update
- make
- make install # will install into ~/.lv2 ... AND/OR....
- sudo make install # will install into /usr/lib/lv2

that's it.
