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


#include <cstdlib>
#include <cmath>
#include <iostream>
#include <cstring>
#include <unistd.h>

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

///////////////////////// MACRO SUPPORT ////////////////////////////////

#define __rt_func __attribute__((section(".rt.text")))
#define __rt_data __attribute__((section(".rt.data")))

///////////////////////// FAUST SUPPORT ////////////////////////////////

#define FAUSTFLOAT float
#ifndef N_
#define N_(String) (String)
#endif
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
#define PITCHBEND_INC 0.00146484375  // 24 / (2^14), +/- 1 octave

#define always_inline inline __attribute__((always_inline))

#ifndef signbit
#define signbit(x) std::signbit(x)
#endif

template<class T> inline T mydsp_faustpower2_f(T x) {return (x * x);}
template<class T> inline T mydsp_faustpower3_f(T x) {return ((x * x) * x);}
template<class T> inline T mydsp_faustpower4_f(T x) {return (((x * x) * x) * x);}
template<class T> inline T mydsp_faustpower5_f(T x) {return ((((x * x) * x) * x) * x);}
template<class T> inline T mydsp_faustpower6_f(T x) {return (((((x * x) * x) * x) * x) * x);}

////////////////////////////// LOCAL INCLUDES //////////////////////////

#include "XPolyMonk.h"        // define struct PortIndex
#include "xmonk.cc"    // dsp class generated by faust -> dsp2cc
#include "stereoverb.cc"    // dsp class generated by faust -> dsp2cc
#include "stereodelay.cc"    // dsp class generated by faust -> dsp2cc

////////////////////////////// PLUG-IN CLASS ///////////////////////////

namespace xmonk {

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

#define VOICES 6

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

PolyVoice::PolyVoice()
{
  for(int i = 0; i<VOICES;i++) {
    xmonk[i] = xmonk::plugin();
  }
}

PolyVoice::~PolyVoice()
{
  for(int i = 0; i<VOICES;i++) {
    xmonk[i]->del_instance(xmonk[i]);
  }
}

void PolyVoice::init_poly(PolyVoice *p, uint32_t rate) {
  for(int i = 0; i<VOICES;i++) {
    p->xmonk[i]->init_static(rate, p->xmonk[i]);
  }
  last_voice = 0;
  velocity = 1.0;
  sustain = 0.5;
}

void PolyVoice::connect_poly(PolyVoice *p, uint32_t port,void* data) {
  for(int i = 0; i<VOICES;i++) {
    p->xmonk[i]->connect_static(port,  data, p->xmonk[i]);
  }
}

void PolyVoice::run_poly(PolyVoice *p, uint32_t n_samples, float* output, float* output1) {
  memset(output,0,n_samples*sizeof(float));
  memset(output1,0,n_samples*sizeof(float));
  for(int i = 0; i<VOICES;i++) {
    p->xmonk[i]->note = (int) p->voices[i] ? (double) p->voices[i] + p->pitchbend : p->xmonk[i]->note;
    p->xmonk[i]->gate = (int) p->voices[i] ? 1.0 : 0.0;
    p->xmonk[i]->vowel = (double) p->vowel;
    p->xmonk[i]->panic = (double) p->panic;
    p->xmonk[i]->velocity = (double) p->velocity;
    p->xmonk[i]->sustain = (double) p->sustain;
    p->xmonk[i]->gain = (double) p->gain;
    p->xmonk[i]->compute_static(static_cast<int>(n_samples), output, output1, p->xmonk[i]);
  }
}

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
  float* gain;
  float pitchbend;
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

  void clear_voice_list();
  void remove_first_voice();
  void add_voice(uint8_t *key);
  void remove_voice(uint8_t *key);

public:
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

// constructor
XPolyMonk_::XPolyMonk_() :
  note(NULL),
  gate(NULL),
  panic(NULL),
  vowel(NULL),
  output(NULL),
  output1(NULL),
  p(),
  reverb(stereoverb::plugin()),
  delay(stereodelay::plugin()) {};

// destructor
XPolyMonk_::~XPolyMonk_()
{
  // delete DSP class
  reverb->del_instance(reverb);
  delay->del_instance(delay);
};

///////////////////////// PRIVATE CLASS  FUNCTIONS /////////////////////

void XPolyMonk_::init_dsp_(uint32_t rate)
{
  p.init_poly(&p, rate);
  reverb->init_static(rate, reverb); // init the DSP class
  delay->init_static(rate, delay); // init the DSP class
  pitchbend = 0.0;
  clear_voice_list();
}

// connect the Ports used by the plug-in class
void XPolyMonk_::connect_(uint32_t port,void* data)
{
  switch ((PortIndex)port)
    {
    case EFFECTS_OUTPUT:
      output = static_cast<float*>(data);
      break;
    case EFFECTS_OUTPUT1:
      output1 = static_cast<float*>(data);
      break;
    case MIDI_IN:
      control = (const LV2_Atom_Sequence*)data;
      break;
    case MIDINOTE:
      note = (float*)data;
      break;
    case MIDIVOWEL:
      vowel = (float*)data;
      break;
    case MIDIGATE:
      gate = (float*)data;
      break;
    case MIDISUSTAIN:
      sustain = (float*)data;
      break;
    case MIDIGAIN:
      gain = (float*)data;
      break;
    case NOTE:
      ui_note = (float*)data;
      break;
    case VOWEL:
      ui_vowel = (float*)data;
      break;
    case GATE:
      ui_gate = (float*)data;
      break;
    case PANIC:
      panic = (float*)data;
      break;
    case SUSTAIN:
      ui_sustain = (float*)data;
      break;
    case GAIN:
      ui_gain = (float*)data;
      break;
    default:
      break;
    }
}

void XPolyMonk_::activate_f()
{
}

void XPolyMonk_::clean_up()
{
}

void XPolyMonk_::deactivate_f()
{
  reverb->clear_state_f_static(reverb);
  delay->clear_state_f_static(delay);
}

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

void XPolyMonk_::run_dsp_(uint32_t n_samples)
{
    if(n_samples<1) return;
    MXCSR.set_();

    if((*ui_sustain) != _ui_sustain) {
        if(!(int)floor((*ui_sustain)) && (int)floor((_ui_sustain)))
            clear_voice_list();
       _ui_sustain = (*ui_sustain);
       (*sustain) = (*ui_sustain);
    }

    if((*ui_vowel) != (_ui_vowel)) {
        _ui_vowel = (*ui_vowel);
        (*vowel) = (*ui_vowel);
    }

    if((*ui_gain) != (_ui_gain)) {
        _ui_gain = (*ui_gain);
        (*gain) = (*ui_gain);
    }

    LV2_ATOM_SEQUENCE_FOREACH(control, ev) {
        if (ev->body.type == midi_MidiEvent) {
            const uint8_t* const msg = (const uint8_t*)(ev + 1);
            switch (lv2_midi_message_type(msg)) {
            case LV2_MIDI_MSG_NOTE_ON:
                note_on = msg[1];
                p.velocity = (float)((msg[2]+1.0)/128.0); 
                (*note) = max(0.0, min((float)note_on + pitchbend, 127.0));
                (*ui_note) = (*note);
                (*gate) = 1.0;
                (*ui_gate) = 1.0;
                (*panic) = 1.0;
                add_voice(&note_on);
            break;
            case LV2_MIDI_MSG_NOTE_OFF:
                note_off = msg[1];
                remove_voice(&note_off);
            break;
            case LV2_MIDI_MSG_CONTROLLER:
                switch (msg[1]) {
                    case LV2_MIDI_CTL_MSB_MODWHEEL:
                    case LV2_MIDI_CTL_LSB_MODWHEEL:
                        (*vowel) = (float) (msg[2]/31.0);
                        (*ui_vowel) = (*vowel);
                    break;
                    case LV2_MIDI_CTL_ALL_SOUNDS_OFF:
                    case LV2_MIDI_CTL_ALL_NOTES_OFF:
                        (*gate) = 0.0;
                        (*ui_gate) = 0.0;
                        (*panic) = 0.0;
                    break;
                    case LV2_MIDI_CTL_RESET_CONTROLLERS:
                        pitchbend = 0.0;
                        (*vowel) = 2.0;
                        (*ui_vowel) = 2.0;
                    break;
                    case LV2_MIDI_CTL_SUSTAIN:
                        (*sustain) = (float) (msg[2]/127.0);
                        (*ui_sustain) = (*sustain);
                    default:
                    case LV2_MIDI_CTL_MSB_MAIN_VOLUME:
                        (*gain) = (float) (msg[2]/127.0);
                        (*ui_gain) = (*gain);
                    break;
                }
            break;
            case LV2_MIDI_MSG_BENDER:
                pitchbend = ((msg[2] << 7 | msg[1]) - 8192) * PITCHBEND_INC;
                (*note) = max(0.0, min((float)note_on + pitchbend, 127.0));
                (*ui_note) = (*note);
            break;
            default:
            break;
            }
        }
    }
    p.vowel = (*vowel);
    p.panic = (*panic);
    p.sustain = (*sustain);
    p.gain = (*gain);
    p.pitchbend = pitchbend;
    
    p.run_poly(&p, n_samples, output, output1);
    reverb->compute_static(static_cast<int>(n_samples), output, output1, output, output1, reverb);
    delay->compute_static(static_cast<int>(n_samples), output, output1, output, output1, delay);
    MXCSR.reset_();
}

void XPolyMonk_::connect_all__ports(uint32_t port, void* data)
{
  // connect the Ports used by the plug-in class
  connect_(port,data); 
  // connect the Ports used by the DSP class
  p.connect_poly(&p, port, data);
  reverb->connect_static(port,  data, reverb);
  delay->connect_static(port,  data, delay);
}

////////////////////// STATIC CLASS  FUNCTIONS  ////////////////////////

LV2_Handle 
XPolyMonk_::instantiate(const LV2_Descriptor* descriptor,
                            double rate, const char* bundle_path,
                            const LV2_Feature* const* features)
{

  LV2_URID_Map* map = NULL;
  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID__map)) {
        map = (LV2_URID_Map*)features[i]->data;
            break;
        }
    }
  if (!map) {
      return NULL;
  }

  // init the plug-in class
  XPolyMonk_ *self = new XPolyMonk_();
  if (!self) {
    return NULL;
  }
  self->map = map;
  self->midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);

  self->init_dsp_((uint32_t)rate);

  return (LV2_Handle)self;
}

void XPolyMonk_::connect_port(LV2_Handle instance, 
                                   uint32_t port, void* data)
{
  // connect all ports
  static_cast<XPolyMonk_*>(instance)->connect_all__ports(port, data);
}

void XPolyMonk_::activate(LV2_Handle instance)
{
  // allocate needed mem
  static_cast<XPolyMonk_*>(instance)->activate_f();
}

void XPolyMonk_::run(LV2_Handle instance, uint32_t n_samples)
{
  // run dsp
  static_cast<XPolyMonk_*>(instance)->run_dsp_(n_samples);
}

void XPolyMonk_::deactivate(LV2_Handle instance)
{
  // free allocated mem
  static_cast<XPolyMonk_*>(instance)->deactivate_f();
}

void XPolyMonk_::cleanup(LV2_Handle instance)
{
  // well, clean up after us
  XPolyMonk_* self = static_cast<XPolyMonk_*>(instance);
  self->clean_up();
  delete self;
}

const LV2_Descriptor XPolyMonk_::descriptor =
{
  PLUGIN_URI,
  XPolyMonk_::instantiate,
  XPolyMonk_::connect_port,
  XPolyMonk_::activate,
  XPolyMonk_::run,
  XPolyMonk_::deactivate,
  XPolyMonk_::cleanup,
  NULL
};


} // end namespace xmonk

////////////////////////// LV2 SYMBOL EXPORT ///////////////////////////

extern "C"
LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  switch (index)
    {
    case 0:
      return &xmonk::XPolyMonk_::descriptor;
    default:
      return NULL;
    }
}

///////////////////////////// FIN //////////////////////////////////////
