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
#include "compressor.cc"    // dsp class generated by faust -> dsp2cc
#include "stereoverb.cc"    // dsp class generated by faust -> dsp2cc
#include "stereodelay.cc"    // dsp class generated by faust -> dsp2cc

////////////////////////////// PLUG-IN CLASS ///////////////////////////

namespace xmonk {

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
  detune = 0.0;
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
    p->xmonk[i]->attack = (double) p->attack;
    p->xmonk[i]->sustain = (double) p->sustain;
    p->xmonk[i]->release = (double) p->release;
    p->xmonk[i]->gain = (double) p->gain;
    p->xmonk[i]->detune = (double) p->detune * i * 0.1;
    p->xmonk[i]->compute_static(static_cast<int>(n_samples), output, output1, p->xmonk[i]);
  }
}

PolyVoice* init_() {
    return new PolyVoice();
}

void PolyVoice::delete_poly(PolyVoice* p) {
    delete p;
}

// constructor
XPolyMonk_::XPolyMonk_() :
  note(NULL),
  gate(NULL),
  panic(NULL),
  attack(NULL),
  release(NULL),
  vowel(NULL),
  detune(NULL),
  sustain(NULL),
  output(NULL),
  output1(NULL),
  p(init_()),
  compress(compressor::plugin()),
  reverb(stereoverb::plugin()),
  delay(stereodelay::plugin()) {};

// destructor
XPolyMonk_::~XPolyMonk_()
{
  // delete DSP class
  p->delete_poly(p);
  compress->del_instance(compress);
  reverb->del_instance(reverb);
  delay->del_instance(delay);
};

///////////////////////// PRIVATE CLASS  FUNCTIONS /////////////////////

void XPolyMonk_::init_dsp_(uint32_t rate)
{
  p->init_poly(p, rate);
  compress->init_static(rate, compress); // init the DSP class
  reverb->init_static(rate, reverb); // init the DSP class
  delay->init_static(rate, delay); // init the DSP class
  pitchbend = 0.0;
  velocity = 1.0;
  _ui_detune = 2.0;
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
    case MIDIDETUNE:
      detune = (float*)data;
      break;
    case MIDIGAIN:
      gain = (float*)data;
      break;
    case MIDIATTACK:
      attack = (float*)data;
      break;
    case MIDIRELEASE:
      release = (float*)data;
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
    case DETUNE:
      ui_detune = (float*)data;
      break;
    case ATTACK: 
      ui_attack = (float*)data; // , 0.0, 0.0, 6.0, 1.0 
      break;
    case RELEASE: 
      ui_release = (float*)data; // , 0.0, 0.0, 6.0, 1.0 
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
  compress->clear_state_f_static(compress);
  reverb->clear_state_f_static(reverb);
  delay->clear_state_f_static(delay);
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

    if((*ui_detune) != (_ui_detune)) {
        _ui_detune = (*ui_detune);
        (*detune) = (*ui_detune);
        fprintf(stderr, "detune = %f\n", (*detune));
    }

    if((*ui_attack) != (_ui_attack)) {
        _ui_attack = (*ui_attack);
        (*attack) = (*ui_attack);
    }

    if((*ui_release) != (_ui_release)) {
        _ui_release = (*ui_release);
        (*release) = (*ui_release);
    }

    if((*ui_gain) != (_ui_gain)) {
        _ui_gain = (*ui_gain);
        (*gain) = (*ui_gain);
    }

    if((*panic) != (_panic)) {
        _panic = (*panic);
        if(!_panic) clear_voice_list();
    }

    LV2_ATOM_SEQUENCE_FOREACH(control, ev) {
        if (ev->body.type == midi_MidiEvent) {
            const uint8_t* const msg = (const uint8_t*)(ev + 1);
            switch (lv2_midi_message_type(msg)) {
            case LV2_MIDI_MSG_NOTE_ON:
                note_on = msg[1];
                velocity = (float)((msg[2]+1.0)/128.0); 
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
                        clear_voice_list();
                    break;
                    case LV2_MIDI_CTL_RESET_CONTROLLERS:
                        pitchbend = 0.0;
                        (*vowel) = 2.0;
                        (*ui_vowel) = 2.0;
                        clear_voice_list();
                    break;
                    case LV2_MIDI_CTL_SUSTAIN:
                        (*sustain) = (float) (msg[2]/127.0);
                        (*ui_sustain) = (*sustain);
                    default:
                    case LV2_MIDI_CTL_SC4_ATTACK_TIME:
                        (*attack) = (float) (msg[2]/127.0);
                        (*ui_attack) = (*gain);
                    break;
                    case LV2_MIDI_CTL_SC3_RELEASE_TIME:
                        (*release) = (float) (msg[2]/127.0);
                        (*ui_release) = (*gain);
                    break;
                    case LV2_MIDI_CTL_MSB_MAIN_VOLUME:
                        (*gain) = (float) (msg[2]/127.0);
                        (*ui_gain) = (*gain);
                    break;
                    case LV2_MIDI_CTL_E4_DETUNE_DEPTH:
                        float v = (float)msg[2];
                        if(v>64.0) v *=1.01;
                        (*detune) = v/64.0 - 1.0;
                        (*ui_detune) = (*detune);
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
    p->vowel = (*vowel);
    p->panic = (*panic);
    p->attack = (*attack);
    p->sustain = (*sustain);
    p->release = (*release);
    p->gain = (*gain);
    p->pitchbend = pitchbend;
    p->velocity = std::max<double>(0.1,velocity);
    p->detune = (*detune);
    
    p->run_poly(p, n_samples, output, output1);
    compress->compute_static(static_cast<int>(n_samples), output, output1, output, output1, compress);
    reverb->compute_static(static_cast<int>(n_samples), output, output1, output, output1, reverb);
    delay->compute_static(static_cast<int>(n_samples), output, output1, output, output1, delay);
    MXCSR.reset_();
}

void XPolyMonk_::connect_all__ports(uint32_t port, void* data)
{
  // connect the Ports used by the plug-in class
  connect_(port,data); 
  // connect the Ports used by the DSP class
  p->connect_poly(p, port, data);
  compress->connect_static(port,  data, compress);
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
