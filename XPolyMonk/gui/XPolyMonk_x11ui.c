
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/instance-access/instance-access.h"

// xwidgets.h includes xputty.h and all defined widgets from Xputty
#include "xwidgets.h"
#include "xmidi_keyboard.h"


#include "./XPolyMonk.h"

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                define controller numbers
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

#define CONTROLS 7

#define PITCHBEND_INC 0.00146484375  // 24 / (2^14), +/- 1 octave

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                the main LV2 handle->XWindow
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

// main window struct
typedef struct {
    xmonk::XPolyMonk_ *xpm;
    void *parentXwindow;
    Xputty main;
    Widget_t *win;
    Widget_t *gain_knob;
    Widget_t *vibrato_knob;
    Widget_t *vowel_knob;
    Widget_t *combobox;
    Widget_t *key_button;
    Widget_t *keyboard;
    Widget_t *detune_knob;
    Widget_t *attack_knob;
    Widget_t *decay_knob;
    Widget_t *break_point_knob;
    Widget_t *slope_knob;
    Widget_t *sustain_knob;
    Widget_t *hold_knob;
    Widget_t *release_knob;
    Widget_t *env_amp_knob;
    Widget_t *voices_combobox;
    MidiKeyboard *keys;
    int block_event;
    float panic;
    float pitchbend;
    float sensity;
    float midi_note;
    float midi_vowel;
    float midi_detune;
    float midi_gate;
    float midi_attack;
    float midi_sustain;
    float midi_release;
    float midi_gain;
    float detune;
    bool ignore_midi_note;
    bool ignore_midi_vowel;
    bool ignore_midi_detune;
    bool ignore_midi_gate;
    bool ignore_midi_attack;
    bool ignore_midi_sustain;
    bool ignore_midi_release;
    bool ignore_midi_gain;

    void *controller;
    LV2UI_Write_Function write_function;
    LV2UI_Resize* resize;
} X11_UI;


// if controller value changed send notify to host
static void value_changed(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    X11_UI* ui = (X11_UI*)w->parent_struct;
    if (ui->block_event != w->data) 
        ui->write_function(ui->controller,w->data,sizeof(float),0,&w->adj->value);
    ui->block_event = -1;
}

// if controller value changed send notify to host
static void adj_changed(Widget_t *w, PortIndex index, float value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    if (ui->block_event != index) 
        ui->write_function(ui->controller,index,sizeof(float),0,&value);
    ui->block_event = -1;
}

static void pm_draw_knob(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width-2;
    int height = attrs.height-2;

    const double scale_zero = 20 * (M_PI/180); // defines "dead zone" for knobs
    int arc_offset = 2;
    int knob_x = 0;
    int knob_y = 0;

    int grow = (width > height) ? height:width;
    knob_x = grow-1;
    knob_y = grow-1;
    /** get values for the knob **/

    int knobx = (width - knob_x) * 0.5;
    int knobx1 = width* 0.5;

    int knoby = (height - knob_y) * 0.5;
    int knoby1 = height * 0.5;

    double knobstate = adj_get_state(w->adj_y);
    double angle = scale_zero + knobstate * 2 * (M_PI - scale_zero);

    double pointer_off =knob_x/3.5;
    double radius = min(knob_x-pointer_off, knob_y-pointer_off) / 2;
    double lengh_x = (knobx+radius+pointer_off/2) - radius * sin(angle);
    double lengh_y = (knoby+radius+pointer_off/2) + radius * cos(angle);
    double radius_x = (knobx+radius+pointer_off/2) - radius/ 1.18 * sin(angle);
    double radius_y = (knoby+radius+pointer_off/2) + radius/ 1.18 * cos(angle);
    cairo_pattern_t* pat;
    cairo_new_path (w->crb);

    pat = cairo_pattern_create_linear (0, 0, 0, knob_y);
    cairo_pattern_add_color_stop_rgba (pat, 1,  0.3, 0.3, 0.3, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 0.75,  0.2, 0.2, 0.2, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 0.5,  0.15, 0.15, 0.15, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 0.25,  0.1, 0.1, 0.1, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 0,  0.05, 0.05, 0.05, 1.0);

    cairo_scale (w->crb, 0.95, 1.05);
    cairo_arc(w->crb,knobx1+arc_offset/2, knoby1-arc_offset, knob_x/2.2, 0, 2 * M_PI );
    cairo_set_source (w->crb, pat);
    cairo_fill_preserve (w->crb);
    cairo_set_source_rgb (w->crb, 0.1, 0.1, 0.1); 
    cairo_set_line_width(w->crb,1);
    cairo_stroke(w->crb);
    cairo_scale (w->crb, 1.05, 0.95);
    cairo_new_path (w->crb);
    cairo_pattern_destroy (pat);
    pat = NULL;

    pat = cairo_pattern_create_linear (0, 0, 0, knob_y);
    cairo_pattern_add_color_stop_rgba (pat, 0,  0.3, 0.3, 0.3, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 0.25,  0.2, 0.2, 0.2, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 0.5,  0.15, 0.15, 0.15, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 0.75,  0.1, 0.1, 0.1, 1.0);
    cairo_pattern_add_color_stop_rgba (pat, 1,  0.05, 0.05, 0.05, 1.0);

    cairo_arc(w->crb,knobx1, knoby1, knob_x/2.6, 0, 2 * M_PI );
    cairo_set_source (w->crb, pat);
    cairo_fill_preserve (w->crb);
    cairo_set_source_rgb (w->crb, 0.1, 0.1, 0.1); 
    cairo_set_line_width(w->crb,1);
    cairo_stroke(w->crb);
    cairo_new_path (w->crb);
    cairo_pattern_destroy (pat);

    /** create a rotating pointer on the kob**/
    cairo_set_line_cap(w->crb, CAIRO_LINE_CAP_ROUND); 
    cairo_set_line_join(w->crb, CAIRO_LINE_JOIN_BEVEL);
    cairo_move_to(w->crb, radius_x, radius_y);
    cairo_line_to(w->crb,lengh_x,lengh_y);
    cairo_set_line_width(w->crb,3);
    cairo_set_source_rgb (w->crb,0.63,0.63,0.63);
    cairo_stroke(w->crb);
    cairo_new_path (w->crb);

    cairo_text_extents_t extents;
    /** show value on the kob**/
    if (w->state) {
        char s[64];
        const char* format[] = {"%.1f", "%.2f", "%.3f"};
        snprintf(s, 63, format[2-1], w->adj_y->value);
        cairo_set_source_rgb (w->crb, 0.6, 0.6, 0.6);
        cairo_set_font_size (w->crb, knobx1/3);
        cairo_select_font_face (w->crb, "Sans", CAIRO_FONT_SLANT_NORMAL,
                                   CAIRO_FONT_WEIGHT_BOLD);
        cairo_text_extents(w->crb, s, &extents);
        cairo_move_to (w->crb, knobx1-extents.width/2, knoby1+extents.height/2);
        cairo_show_text(w->crb, s);
        cairo_new_path (w->crb);
    }

    /** show label below the knob**/
    use_text_color_scheme(w, get_color_state(w));
    float font_size = ((height/2.2 < (width*0.5)/3) ? height/2.2 : (width*0.5)/3);
    cairo_set_font_size (w->crb, font_size);
    cairo_select_font_face (w->crb, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
    cairo_text_extents(w->crb,w->label , &extents);

    cairo_move_to (w->crb, knobx1-extents.width/2, height );
    cairo_show_text(w->crb, w->label);
    cairo_new_path (w->crb);
}

// draw the window
static void draw_window(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    set_pattern(w,&w->app->color_scheme->selected,&w->app->color_scheme->normal,BACKGROUND_);
    cairo_paint (w->crb);
    widget_set_scale(w);
    use_text_color_scheme(w, get_color_state(w));
    cairo_set_font_size (w->crb, 25);
    cairo_select_font_face (w->crb, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to (w->crb, 75, 35);
    cairo_show_text(w->crb, w->label);
    widget_reset_scale(w);
}

static void reset_panic(X11_UI* ui) {
    if(ui->panic<0.1) {
        ui->panic = 1.0;
        ui->write_function(ui->controller,PANIC,sizeof(float),0,&ui->panic);
    }
}

static void get_note(Widget_t *w, int *key, bool on_off) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    if (on_off) {
        reset_panic(ui);
        
        ui->xpm->add_voice((int)ui->voices_combobox->adj->value,(uint8_t*)key);
          
    } else {
        ui->xpm->remove_voice((uint8_t*)key);
    }
}

static void get_pitch(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    ui->xpm->pitchbend = (float)((*value) -64.0) * ui->sensity * PITCHBEND_INC;
    
}

static void get_sensity(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    ui->sensity = (float)(*value);
}

static void get_mod(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    float v = (float)(*value)/32.0;
    check_value_changed(ui->vowel_knob->adj, &v);
}

static void get_detune(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    if((*value)>64) (*value) *=1.01;
    float v = (float)(*value)/64.0 - 1.0;
    check_value_changed(ui->detune_knob->adj, &v);
}

static void get_volume(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    float v = (float)(*value)/127.0;
    check_value_changed(ui->gain_knob->adj, &v);
}

static void get_velocity(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    ui->xpm->velocity = (float)(((*value)+1.0)/128.0); 
}

static void get_attack(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    float v = (float)(*value)/127.0;
    check_value_changed(ui->attack_knob->adj, &v);
}

static void get_sustain(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    float v = (float)(*value)/127.0;
    check_value_changed(ui->sustain_knob->adj, &v);
}

static void get_release(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    float v = (float)(*value)/127.0;
    check_value_changed(ui->release_knob->adj, &v);
}

static void get_all_sound_off(Widget_t *w,int *value) {
    X11_UI* ui = (X11_UI*)w->parent_struct;
    adj_changed(w, GATE, 0.0);
    ui->panic = 0.0;
    ui->write_function(ui->controller,PANIC,sizeof(float),0,&ui->panic);
}

static void win_key_release(void *w_, void *key_, void *user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    XKeyEvent *key = (XKeyEvent*)key_;
    KeySym sym = XLookupKeysym (key, 0);
    if (sym == XK_space) {
        get_all_sound_off(w, NULL);
    }
}

static void key_button_callback(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    X11_UI* ui = (X11_UI*)p->parent_struct;
    
    if (w->flags & HAS_POINTER && adj_get_value(w->adj)){
        widget_show_all(ui->keyboard);
        tooltip_set_text(w, "Hide Virtual Keyboard");
    }
    if (w->flags & HAS_POINTER && !adj_get_value(w->adj)){
        widget_hide(ui->keyboard);
        tooltip_set_text(w, "Show Virtual Keyboard");
    }
}

static void keyboard_hidden(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *p = (Widget_t*)w->parent;
    X11_UI* ui = (X11_UI*)p->parent_struct;
    adj_set_value(ui->key_button->adj,0.0);
}

static void dummy_event(void *w_, void* user_data) {
    // just overwite event with nothing
}

Widget_t* add_polymonk_knob(Widget_t *w, PortIndex index, const char * label,
                                X11_UI* ui, int x, int y, int width, int height) {
    w = add_knob(ui->win, label, x, y, width, height);
    w->func.expose_callback = pm_draw_knob;    
    //w->flags |= FAST_REDRAW;
    //w->scale.gravity = CENTER;
    set_adjustment(w->adj,0.0, 0.0, 0.0, 1.0, 0.005, CL_CONTINUOS);
    w->parent_struct = ui;
    w->data = index;
    w->func.value_changed_callback = value_changed;
    return w;
}

// init the xwindow and return the LV2UI handle
static LV2UI_Handle instantiate(const struct LV2UI_Descriptor * descriptor,
            const char * plugin_uri, const char * bundle_path,
            LV2UI_Write_Function write_function,
            LV2UI_Controller controller, LV2UI_Widget * widget,
            const LV2_Feature * const * features) {

    X11_UI* ui = (X11_UI*)malloc(sizeof(X11_UI));

    if (!ui) {
        fprintf(stderr,"ERROR: failed to instantiate plugin with URI %s\n", plugin_uri);
        return NULL;
    }

    ui->parentXwindow = 0;
    ui->xpm = NULL;
    LV2UI_Resize* resize = NULL;
    ui->block_event = -1;
    bool instance_access = false;

    int i = 0;
    for (; features[i]; ++i) {
        if (!strcmp(features[i]->URI, LV2_UI__parent)) {
            ui->parentXwindow = features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_UI__resize)) {
            resize = (LV2UI_Resize*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_INSTANCE_ACCESS_URI) ) {
            ui->xpm = (xmonk::XPolyMonk_ *)(features[i]->data);
        }
    }

    if (ui->parentXwindow == NULL)  {
        fprintf(stderr, "ERROR: Failed to open parentXwindow for %s\n", plugin_uri);
        free(ui);
        return NULL;
    }

    if (ui->xpm) {
        instance_access = true;
    } else {
        fprintf(stderr, "ERROR: Failed to get instance access %s\n", plugin_uri);
    }

    ui->pitchbend = 0.0;
    ui->sensity = 64.0;
    ui->panic = 1.0;
    ui->midi_note = 0.0;
    ui->midi_vowel = 0.0;
    ui->midi_attack = 0.0;
    ui->midi_sustain = 0.0;
    ui->midi_release = 0.0;
    ui->midi_detune = 0.0;
    ui->midi_gate = 0.0;
    ui->midi_gain = 0.0;
    ui->midi_detune = 0.0;
    ui->ignore_midi_note = true;
    ui->ignore_midi_vowel = true;
    ui->ignore_midi_detune = true;
    ui->ignore_midi_gate = true;
    ui->ignore_midi_attack = true;
    ui->ignore_midi_sustain = true;
    ui->ignore_midi_release = true;
    ui->ignore_midi_gain = true;
    // init Xputty
    main_init(&ui->main);
    // create the toplevel Window on the parentXwindow provided by the host
    ui->win = create_window(&ui->main, (Window)ui->parentXwindow, 0, 0, 800, 140);
     // store a pointer to the X11_UI struct in the parent_struct Widget_t field
    ui->win->parent_struct = ui;
    ui->win->label = "XPolyMonk";
    // note
    ui->win->adj_y = add_adjustment(ui->win,40.0, 40.0, 28.0, 52.0, 0.1, CL_NONE);
    ui->win->func.key_release_callback = win_key_release;
    // connect the expose func
    ui->win->func.expose_callback = draw_window;

    // create slider widgets
    ui->gain_knob = add_polymonk_knob(ui->gain_knob, GAIN, "Gain", ui, 5, 50, 70, 85);
    ui->vibrato_knob = add_polymonk_knob(ui->vibrato_knob, VIBRATO,"Vibrato", ui, 75, 50, 70, 85);
    set_adjustment(ui->vibrato_knob->adj,6.0, 6.0, 0.0, 12.0, 0.1, CL_CONTINUOS);

    ui->vowel_knob = add_polymonk_knob(ui->vowel_knob, VOWEL,"Vowel", ui, 145, 50, 70, 85);
    set_adjustment(ui->vowel_knob->adj,2.0, 2.0, 0.0, 4.0, 0.02, CL_CONTINUOS);

    ui->detune_knob = add_polymonk_knob(ui->win, DETUNE, "Detune", ui, 215, 50, 70, 85);
    set_adjustment(ui->detune_knob->adj,0.0, 0.0, -1.0, 1.0, 0.01, CL_CONTINUOS);

    ui->attack_knob = add_polymonk_knob(ui->attack_knob, ATTACK, "Attack", ui, 310, 55, 60, 75);
    ui->decay_knob = add_polymonk_knob(ui->decay_knob, DECAY, "Decay", ui, 370, 55, 60, 75);
    ui->break_point_knob = add_polymonk_knob(ui->break_point_knob, BREAK_POINT, "Break", ui, 430, 55, 60, 75);
    ui->slope_knob = add_polymonk_knob(ui->slope_knob, SLOPE, "Slope", ui, 490, 55, 60, 75);
    ui->sustain_knob = add_polymonk_knob(ui->sustain_knob, SUSTAIN, "Sustain", ui, 550, 55, 60, 75);
    ui->hold_knob = add_polymonk_knob(ui->hold_knob, HOLD, "Hold", ui, 610, 55, 60, 75);
    ui->release_knob = add_polymonk_knob(ui->release_knob, RELEASE, "Release", ui, 670, 55, 60, 75);

    ui->env_amp_knob = add_polymonk_knob(ui->win, ENV_AMP, "Env Amp", ui, 730, 55, 60, 75);
    set_adjustment(ui->env_amp_knob->adj,1.0, 1.0, 0.5, 2.0, 0.01, CL_CONTINUOS);


    // only enable internal keyboard when we've instance access
    if (instance_access) {
        ui->key_button = add_image_toggle_button(ui->win, "Keyboard", 15, 10, 30, 30);
        add_tooltip(ui->key_button, "Show Virtual Keyboard");
        ui->key_button->scale.gravity = ASPECT;
        widget_get_png(ui->key_button, LDVAR(midikeyboard_png));
        ui->key_button->func.value_changed_callback = key_button_callback;
    }
    // create combobox widget
    ui->voices_combobox = add_hslider(ui->win, "Voices", 500, 10, 200, 40);
    ui->voices_combobox->scale.gravity = ASPECT;
    set_adjustment(ui->voices_combobox->adj,1.0, 1.0, 1.0, 12.0, 1.0, CL_CONTINUOS);
   // combobox_set_active_entry(ui->voices_combobox, 6);
    // store the port index in the Widget_t data field
    ui->voices_combobox->data = VOICE;
    // store a pointer to the X11_UI struct in the parent_struct Widget_t field
    ui->voices_combobox->parent_struct = ui;
    // connect the value changed callback with the write_function
    ui->voices_combobox->func.value_changed_callback = value_changed;

    // create combobox widget
    ui->combobox = add_combobox(ui->win, "", 700, 10, 90, 30);
    ui->combobox->scale.gravity = ASPECT;
    combobox_add_entry(ui->combobox,"---");
    combobox_add_entry(ui->combobox,"12-ET");
    combobox_add_entry(ui->combobox,"19-ET");
    combobox_add_entry(ui->combobox,"24-ET");
    combobox_add_entry(ui->combobox,"31-ET");
    combobox_add_entry(ui->combobox,"41-ET");
    combobox_add_entry(ui->combobox,"53-ET");
    combobox_set_active_entry(ui->combobox, 0);
    // store the port index in the Widget_t data field
    ui->combobox->data = SCALE;
    // store a pointer to the X11_UI struct in the parent_struct Widget_t field
    ui->combobox->parent_struct = ui;
    // connect the value changed callback with the write_function
    ui->combobox->func.value_changed_callback = value_changed;

    ui->keyboard = open_midi_keyboard(ui->win);
    ui->keyboard->flags |= HIDE_ON_DELETE;
    ui->keyboard->func.unmap_notify_callback = keyboard_hidden;
    ui->keyboard->func.map_notify_callback = dummy_event;
    ui->keys = (MidiKeyboard*)ui->keyboard->parent_struct;
    ui->keys->mk_send_note = get_note;
    ui->keys->mk_send_pitch = get_pitch;
    ui->keys->mk_send_pitchsensity = get_sensity;
    ui->keys->mk_send_mod = get_mod;
    ui->keys->mk_send_detune = get_detune;
    ui->keys->mk_send_volume = get_volume;
    ui->keys->mk_send_velocity = get_velocity;
    ui->keys->mk_send_attack = get_attack;
    ui->keys->mk_send_sustain = get_sustain;
    ui->keys->mk_send_release = get_release;
    ui->keys->mk_send_all_sound_off = get_all_sound_off;
    
    // finally map all Widgets on screen
    widget_show_all(ui->win);
    // set the widget pointer to the X11 Window from the toplevel Widget_t
    *widget = (void*)ui->win->widget;
    // request to resize the parentXwindow to the size of the toplevel Widget_t
    if (resize){
        ui->resize = resize;
        resize->ui_resize(resize->handle, 800, 140);
    }
    // store pointer to the host controller
    ui->controller = controller;
    // store pointer to the host write function
    ui->write_function = write_function;
    
    return (LV2UI_Handle)ui;
}

// cleanup after usage
static void cleanup(LV2UI_Handle handle) {
    X11_UI* ui = (X11_UI*)handle;
    // Xputty free all memory used
    main_quit(&ui->main);
    free(ui);
}

/*---------------------------------------------------------------------
-----------------------------------------------------------------------    
                        LV2 interface
-----------------------------------------------------------------------
----------------------------------------------------------------------*/

// port value change message from host
static void port_event(LV2UI_Handle handle, uint32_t port_index,
                        uint32_t buffer_size, uint32_t format,
                        const void * buffer) {
    X11_UI* ui = (X11_UI*)handle;
    float value = *(float*)buffer;
    switch (port_index) {
        case MIDIVOWEL:
            if (ui->ignore_midi_vowel) {
                ui->ignore_midi_vowel = false;
                return;
            }
            if (ui->midi_vowel != value) {
                if(value>-0.1 && value<4.1) {
                    check_value_changed(ui->vowel_knob->adj, &value);
                    // prevent event loop between host and plugin
                    ui->block_event = VOWEL;
                    ui->midi_vowel = value;
                }
            }
        break;    
        case MIDIDETUNE:
            if (ui->ignore_midi_detune) {
                ui->ignore_midi_detune = false;
                return;
            }
            if (ui->midi_detune != value) {
                if(value>-1.1 && value<1.1) {
                    check_value_changed(ui->detune_knob->adj, &value);
                    // prevent event loop between host and plugin
                    ui->block_event = DETUNE;
                    ui->midi_detune = value;
                }
            }
        break;
        case MIDINOTE:
            if (ui->ignore_midi_note) {
                ui->ignore_midi_note = false;
                return;
            }
            if (ui->midi_note != value) {
                if(value >-1.0 && value<127.0) {
                    check_value_changed(ui->win->adj_y, &value);
                    // prevent event loop between host and plugin
                    ui->block_event = NOTE;
                    ui->midi_note = value;
                }
            }
        break;
        case MIDIGATE:
            if (ui->ignore_midi_gate) {
                ui->ignore_midi_gate = false;
                return;
            }
            if (ui->midi_gate != value) {
                if(value>-0.1 && value<1.1) {
                    adj_changed(ui->win, GATE, value);
                    ui->midi_gate = value;
                }
            }
        break;
        case MIDIATTACK:
            if (ui->ignore_midi_attack) {
                ui->ignore_midi_attack = false;
                return;
            }
            if (ui->midi_attack != value) {
                if(value>-0.1 && value<1.1) {
                    check_value_changed(ui->attack_knob->adj, &value);
                    // prevent event loop between host and plugin
                    ui->block_event = ATTACK;
                    ui->midi_attack = value;
                }
            }
        break;
        case MIDISUSTAIN:
            if (ui->ignore_midi_sustain) {
                ui->ignore_midi_sustain = false;
                return;
            }
            if (ui->midi_sustain != value) {
                if(value>-0.1 && value<1.1) {
                    check_value_changed(ui->sustain_knob->adj, &value);
                    // prevent event loop between host and plugin
                    ui->block_event = SUSTAIN;
                    ui->midi_sustain = value;
                }
            }
        break;
        case MIDIRELEASE:
            if (ui->ignore_midi_release) {
                ui->ignore_midi_release = false;
                return;
            }
            if (ui->midi_release != value) {
                if(value>-0.1 && value<1.1) {
                    check_value_changed(ui->release_knob->adj, &value);
                    // prevent event loop between host and plugin
                    ui->block_event = RELEASE;
                    ui->midi_release = value;
                }
            }
        break;
        case MIDIGAIN:
            if (ui->ignore_midi_gain) {
                ui->ignore_midi_gain = false;
                return;
            }
            if (ui->midi_gain != value) {
                if(value>-0.1 && value<1.1) {
                    check_value_changed(ui->gain_knob->adj, &value);
                    // prevent event loop between host and plugin
                    ui->block_event = GAIN;
                    ui->midi_gain = value;
                }
            }
        break;
        case GAIN:
            check_value_changed(ui->gain_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case VIBRATO:
            check_value_changed(ui->vibrato_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case VOWEL:
            check_value_changed(ui->vowel_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case NOTE:
            check_value_changed(ui->win->adj_y, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case GATE:
            adj_changed(ui->win, GATE, value);
        break;
        case SCALE:
            check_value_changed(ui->combobox->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case ATTACK:
            check_value_changed(ui->attack_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case DECAY:
            check_value_changed(ui->decay_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case BREAK_POINT:
            check_value_changed(ui->break_point_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case SLOPE:
            check_value_changed(ui->slope_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case SUSTAIN:
            check_value_changed(ui->sustain_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case HOLD:
            check_value_changed(ui->hold_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case RELEASE:
            check_value_changed(ui->release_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case ENV_AMP:
            check_value_changed(ui->env_amp_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case VOICE:
            check_value_changed(ui->voices_combobox->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case DETUNE:
            check_value_changed(ui->detune_knob->adj, &value);
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        case PANIC:
            ui->panic = value;
            // prevent event loop between host and plugin
            ui->block_event = (int)port_index;
        break;
        default:
        break;
    }
}

// LV2 idle interface to host
static int ui_idle(LV2UI_Handle handle) {
    X11_UI* ui = (X11_UI*)handle;
    // Xputty event loop setup to run one cycle when called
    run_embedded(&ui->main);
    return 0;
}

// LV2 resize interface to host
static int ui_resize(LV2UI_Feature_Handle handle, int w, int h) {
    X11_UI* ui = (X11_UI*)handle;
    // Xputty sends configure event to the toplevel widget to resize itself
    if (ui) send_configure_event(ui->win,0, 0, w, h);
    return 0;
}

// connect idle and resize functions to host
static const void* extension_data(const char* uri) {
    static const LV2UI_Idle_Interface idle = { ui_idle };
    static const LV2UI_Resize resize = { 0 ,ui_resize };
    if (!strcmp(uri, LV2_UI__idleInterface)) {
        return &idle;
    }
    if (!strcmp(uri, LV2_UI__resize)) {
        return &resize;
    }
    return NULL;
}

static const LV2UI_Descriptor descriptor = {
    PLUGIN_UI_URI,
    instantiate,
    cleanup,
    port_event,
    extension_data
};

extern "C"
LV2_SYMBOL_EXPORT
const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
    switch (index) {
        case 0:
            return &descriptor;
        default:
        return NULL;
    }
}


