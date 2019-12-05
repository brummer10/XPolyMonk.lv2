/*
 * Copyright (C) 2014 Guitarix project MOD project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * --------------------------------------------------------------------------
 */

#pragma once

#ifndef SRC_HEADERS_GXEFFECTS_H_
#define SRC_HEADERS_GXEFFECTS_H_

#include <lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>

#define PLUGIN_URI "https://github.com/brummer10/XPolyMonk.lv2"
#define PLUGIN_UI_URI "https://github.com/brummer10/XPolyMonk_gui"

#define FAUSTFLOAT float
#include "stereodelay.h"
#include "stereoverb.h"
#include "xmonk.h"

typedef enum
{
   EFFECTS_OUTPUT,
   EFFECTS_OUTPUT1,
   NOTE,
   GAIN,
   GATE,
   VOWEL,
   MIDI_IN,
   SCALE,
   SUSTAIN,
   PANIC,
   MIDINOTE,
   MIDIVOWEL,
   MIDIGATE,
   MIDISUSTAIN,
   MIDIGAIN,
} PortIndex;

namespace xmonk {

///////////////////////// DENORMAL PROTECTION WITH SSE /////////////////

#ifdef NOSSE
#undef __SSE__
#endif

#ifdef __SSE__
#include <immintrin.h>
#ifndef _IMMINTRIN_H_INCLUDED
#include <fxsrintrin.h>
#endif
/* On Intel set FZ (Flush to Zero) and DAZ (Denormals Are Zero)
   flags to avoid costly denormals */
#ifdef __SSE3__
#ifndef _PMMINTRIN_H_INCLUDED
#include <pmmintrin.h>
#endif
#else
#ifndef _XMMINTRIN_H_INCLUDED
#include <xmmintrin.h>
#endif
#endif //__SSE3__

#endif //__SSE__

#define VOICES 6

class DenormalProtection
{
private:
#ifdef __SSE__
  uint32_t  mxcsr_mask;
  uint32_t  mxcsr;
  uint32_t  old_mxcsr;
#endif

public:
  inline void set_() {
#ifdef __SSE__
    old_mxcsr = _mm_getcsr();
    mxcsr = old_mxcsr;
    _mm_setcsr((mxcsr | _MM_DENORMALS_ZERO_MASK | _MM_FLUSH_ZERO_MASK) & mxcsr_mask);
#endif
  };
  inline void reset_() {
#ifdef __SSE__
    _mm_setcsr(old_mxcsr);
#endif
  };

  inline DenormalProtection()
  {
#ifdef __SSE__
    mxcsr_mask = 0xffbf; // Default MXCSR mask
    mxcsr      = 0;
    uint8_t fxsave[512] __attribute__ ((aligned (16))); // Structure for storing FPU state with FXSAVE command

    memset(fxsave, 0, sizeof(fxsave));
    __builtin_ia32_fxsave(&fxsave);
    uint32_t mask = *(reinterpret_cast<uint32_t *>(&fxsave[0x1c])); // Obtain the MXCSR mask from FXSAVE structure
    if (mask != 0)
        mxcsr_mask = mask;
#endif
  };

  inline ~DenormalProtection() {};
};

////////////////////////////// PLUG-IN CLASS ///////////////////////////

class PolyVoice
{
private:
  Dsp*      xmonk[VOICES];

public:
  uint8_t voices[VOICES];
  int last_voice;
  float vowel;
  float panic;
  float pitchbend;
  float velocity;
  float sustain;
  float gain;

  void init_poly(PolyVoice *p, uint32_t rate);
  void connect_poly(PolyVoice *p, uint32_t port,void* data);
  void run_poly(PolyVoice *p, uint32_t n_samples, float* output, float* output1);

  PolyVoice();
  ~PolyVoice();
};


class XPolyMonk_
{
private:
  const LV2_Atom_Sequence* control;
  LV2_URID midi_MidiEvent;
  LV2_URID_Map* map;
  uint8_t note_on;
  uint8_t note_off;
  float* note;
  float* gate;
  float* panic;
  float _panic;
  float* gain;
  float* vowel;
  float* ui_note;
  float* ui_gate;
  float* ui_vowel;
  float _ui_vowel;
  float* ui_sustain;
  float _ui_sustain;
  float* sustain;
  float* ui_gain;
  float _ui_gain;

  DenormalProtection MXCSR;
  // pointer to buffer
  float*          output;
  float*          output1;
  // pointer to dsp class
  PolyVoice p;
  stereoverb::Dsp*      reverb;
  stereodelay::Dsp*      delay;

  // private functions
  inline void run_dsp_(uint32_t n_samples);
  inline void connect_(uint32_t port,void* data);
  inline void init_dsp_(uint32_t rate);
  inline void connect_all__ports(uint32_t port, void* data);
  inline void activate_f();
  inline void clean_up();
  inline void deactivate_f();


public:
  void clear_voice_list();
  void remove_first_voice();
  void add_voice(uint8_t *key);
  void remove_voice(uint8_t *key);
  float pitchbend;

  // LV2 Descriptor
  static const LV2_Descriptor descriptor;
  // static wrapper to private functions
  static void deactivate(LV2_Handle instance);
  static void cleanup(LV2_Handle instance);
  static void run(LV2_Handle instance, uint32_t n_samples);
  static void activate(LV2_Handle instance);
  static void connect_port(LV2_Handle instance, uint32_t port, void* data);
  static LV2_Handle instantiate(const LV2_Descriptor* descriptor,
                                double rate, const char* bundle_path,
                                const LV2_Feature* const* features);
  XPolyMonk_();
  ~XPolyMonk_();
};

///////////////////////// PUBLIC CLASS FUNCTIONS USED BY UI /////////////////////

void XPolyMonk_::clear_voice_list() {
    int i = 0;
    for(;i<VOICES;i++) {
        p.voices[i] = 0;
    }
}

void XPolyMonk_::remove_first_voice() {
    int i = 0;
    for(;i<VOICES-1;i++) {
        p.voices[i] = p.voices[i+1];
    }
    p.voices[i] = 0;
}

void XPolyMonk_::add_voice(uint8_t *key) {
    int i = p.last_voice;
    bool set_key = false;
    for(;i<VOICES;i++) {
        if(p.voices[i] == 0) {
            p.voices[i] = (*key);
            set_key = true;
            p.last_voice = i+1;
            break;
        }
    }
    if(!set_key) {
        i = 0;
        for(;i<VOICES;i++) {
            if(p.voices[i] == 0) {
                p.voices[i] = (*key);
                set_key = true;
                p.last_voice = i;
                break;
            }
        }
    }
    if(!set_key) {
        remove_first_voice();
        add_voice(key);
    }
}

void XPolyMonk_::remove_voice(uint8_t *key) {
    int i = 0;
    for(;i<VOICES;i++) {
        if(p.voices[i] == (*key)) {
            p.voices[i] = 0;
            break;
        }
    }
}

} // end namespace xmonk

#endif //SRC_HEADERS_GXEFFECTS_H_
