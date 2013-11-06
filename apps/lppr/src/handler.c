/*
  handler.c
  lppr
  aleph-avr32

 */

//asf
#include "print_funcs.h"

// avr32
#include "control.h"
#include "events.h"
#include "event_types.h"
#include "interrupts.h"
#include "monome.h"
#include "screen.h"
// lppr
#include "app_timers.h"
#include "grid.h"
#include "handler.h"
#include "ctl.h"
#include "render.h"

// keep track of the last touched UI element (when appropriate)
static etype touched = kNumEventTypes;
static u8 touchedThis = 0; // tmp

//---------------------------------------------
//--------- static funcs

// full-scale
static s32 scale_knob_value(s32 val) {
  static const u32 kNumKnobScales_1 = 23;
  static const u32 knobScale[24] = {
    ///--- 3 linear segments:
    // slope = 1
    0x00000001, // 1
    0x00000002, // 2
    // slope = 0x10
    0x00000030, // 3
    0x00000030, // 4
    0x00000040, // 5
    0x00000050, // 6
    0x00000060, // 7
    0x00000070, // 8
    0x00000080, // 9
    0x00000090 ,  // 10
    0x000000a0 , // 11
    0x000000b0 , // 12
    // slope = 0x100
    0x00000c00 , // 13
    0x00000d00 , // 14
    0x00000e00 , // 15
    0x00000f00 , // 16
    0x00001000 , // 17
    0x00001100 , // 18
    0x00001200 , // 19
    0x00001300 , // 20
    0x00001400 , // 21
    0x00001500 , // 22
    // ultra fast
    0x10000000 , // 23
    0x20000000 , // 24
  };
  s32 vabs = BIT_ABS(val);
  s32 ret = val;

  if(vabs > kNumKnobScales_1) {
    vabs = kNumKnobScales_1;
  }
  ret = knobScale[vabs - 1];
  if(val < 0) {
    ret = BIT_NEG_ABS(ret);
  }
  return ret;
}


// lower slope
/* static s32 scale_knob_value_small(s32 val) { */
/*   static const u32 kNumKnobScales_1 = 23; */
/*   static const u32 knobScale[24] = { */
/*     ///--- 3 linear segments: */
/*     // slope = 1 */
/*     0x00000001, // 1 */
/*     0x00000002, // 2 */
/*     // slope = 0x10 */
/*     0x00000030, // 3 */
/*     0x00000030, // 4 */
/*     0x00000040, // 5 */
/*     0x00000050, // 6 */
/*     0x00000060, // 7 */
/*     0x00000070, // 8 */
/*     0x00000080, // 9 */
/*     0x00000090 ,  // 10 */
/*     0x000000a0 , // 11 */
/*     0x000000b0 , // 12 */
/*     0x000000c0 , // 13 */
/*     0x000000d0 , // 14 */
/*     0x000000e0 , // 15 */
/*     0x000000f0 , // 16 */
/*     // slope == 0x100 */
/*     0x00000100 , // 17 */
/*     0x00000200 , // 18 */
/*     0x00000300 , // 19 */
/*     0x00000400 , // 20 */
/*     0x00000500 , // 21 */
/*     0x00000600 , // 22 */
/*     0x00000700 , // 23 */
/*     0x00000800 , // 24 */
/*   }; */

/*   s32 vabs = BIT_ABS(val); */
/*   s32 ret = val; */

/*   if(vabs > kNumKnobScales_1) { */
/*     vabs = kNumKnobScales_1; */
/*   } */
/*   ret = knobScale[vabs - 1]; */
/*   if(val < 0) { */
/*     ret = BIT_NEG_ABS(ret); */
/*   } */
/*   return ret; */
/* } */

  // clear the main region if this is a new touch
static inline void check_touch(etype et) {
  if(touched != et) {
    touchedThis = 1;
    touched = et;
  }
}

static void handle_Switch0(s32 data) {
  check_touch(kEventSwitch0);
  /// tap delaytime 1
  if(data > 0) {
    if(touchedThis) {
      render_touched_delaytime(0);
    }
    ctl_tap_delay(0, data);
  }
  render_sw_on(0, data > 0);
}

static void handle_Switch1(s32 data) {
  check_touch(kEventSwitch1);
  // tap delaytime 2
  if(data > 0) {
    if(touchedThis) {
      render_touched_delaytime(1);
    }	
    ctl_tap_delay(1, data);
  }
  render_sw_on(1, data > 0);
}

static void handle_Switch2(s32 data) {
  //  check_touch(kEventSwitch2);
  if(data > 0) {
    //    if(touchedThis) { ;; }
    // record loop on line 1
    ctl_loop_record(1);
  } 
  render_sw_on(2, data > 0);
}

static void handle_Switch3(s32 data) {
  //  check_touch(kEventSwitch3);
  if(data > 0) {
    if(touchedThis) { ;; }
    // record loop on line 2
    ctl_loop_playback(1);
  }
  render_sw_on(3, data > 0);
}
    
static void handle_Switch6(s32 data) {
  check_touch(kEventSwitch6);
  //    print_dbg("wtf footswitch 1");
  if(data > 0) {
    ctl_loop_record(1);
  } 
  render_sw_on(2, data > 0);
}
    
static void handle_Switch7(s32 data) {
  check_touch(kEventSwitch7);
  //    print_dbg("wtf footswitch 2");
  if(data > 0) {
    ctl_loop_playback(1);
  }
  render_sw_on(3, data > 0);
}
    
static void handle_Encoder0(s32 data) {
  check_touch(kEventEncoder0);
  if(touchedThis) {
    render_touched_fb(0);
  }
  ctl_inc_fb(0, scale_knob_value(data));
}

static void handle_Encoder1(s32 data) {
  check_touch(kEventEncoder1);
  if(touchedThis) {
    render_touched_mix(0);
  }
  ctl_inc_mix(0, scale_knob_value(data));
}

static void handle_Encoder2(s32 data) {
  check_touch(kEventEncoder2);
  if(touchedThis) {
    render_touched_freq(0);
  }
  ctl_inc_freq(0, scale_knob_value(data));
}

static void handle_Encoder3(s32 data) {
  check_touch(kEventEncoder3);
  if(touchedThis) {
    render_touched_res(0);
  }
  ctl_inc_res(0, scale_knob_value(data));
}

//-------- grid

static void handle_MonomeGridKey(s32 data) {
  grid_handle_key_event(data);
}

static void handle_MonomeConnect(s32 data) {
  eMonomeDevice dev;
  u8 w;
  u8 h;
  monome_connect_parse_event_data((u32)data, &dev, &w, &h);
  if(dev != eDeviceGrid) {
    print_dbg("\r\n LPPR monome connect: unsupported device");
    return;
  }
  print_dbg("\r\n LPPR: connecting grid device");
  grid_set_size(w, h);
  timers_set_monome();

  (*monome_set_intense)(0);
}

static void handle_MonomeDisconnect(s32 data) {
  timers_unset_monome();
}


//--------------------------------
//----- extern
void lppr_assign_event_handlers(void) {
  app_event_handlers[ kEventEncoder0 ]	= &handle_Encoder0 ;
  app_event_handlers[ kEventEncoder1 ]	= &handle_Encoder1 ;
  app_event_handlers[ kEventEncoder2 ]	= &handle_Encoder2 ;
  app_event_handlers[ kEventEncoder3 ]	= &handle_Encoder3 ;
  //// FIXME: use mode / power keys
  app_event_handlers[ kEventSwitch0 ]	= &handle_Switch0 ;
  app_event_handlers[ kEventSwitch1 ]	= &handle_Switch1 ;
  app_event_handlers[ kEventSwitch2 ]	= &handle_Switch2 ;
  app_event_handlers[ kEventSwitch3 ]	= &handle_Switch3 ;
  //// FIXME: use mode / power keys
  //  app_event_handlers[ kEventSwitch4 ]	= &handle_Switch4 ;
  //  app_event_handlers[ kEventSwitch5 ]	= &handle_Switch5 ;
  app_event_handlers[ kEventSwitch6 ]	= &handle_Switch6 ;
  app_event_handlers[ kEventSwitch7 ]	= &handle_Switch7 ;
  app_event_handlers[ kEventMonomeConnect ]	= &handle_MonomeConnect ;
  app_event_handlers[ kEventMonomeDisconnect ]	= &handle_MonomeDisconnect ;
  app_event_handlers[ kEventMonomeGridKey ]	= &handle_MonomeGridKey ;
}
