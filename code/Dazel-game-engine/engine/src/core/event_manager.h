#pragma once 
#include "defines.h"
#include "containers/Darray.h"
// set an evemt context
typedef struct event_context{
    // each are 128 bytes
  union 
  {
     i64    i64_byte[2];
     u64    u64_byte[2];
     double f64_byte[2];

     int    i32_byte[4];
     u32    u32_byte[4];
     float  f32_byte[4];

     i16    i16_bytes[8];
     u16    u16_bytes[8];

     char c[16];

  };
  
}event_context;

bool event_intialize();

void event_shutdown();

typedef bool (*PFN_on_event)(u16 code, void*sender, void*listener,event_context data);
 
// register an event to listener to listen to that event
EXP bool event_register(u16 code, void*listener, PFN_on_event on_event);

EXP bool event_unregister(u16 code, void*listener, PFN_on_event on_event);

// unregister that event from the listener
EXP bool event_fire(u16 code, void*sender, event_context data);
// fire the event 

enum system_event_code{
    EVENT_CODE_APPLICATION_QUIT = 0X0001,

    EVENT_CODE_KEY_PRESSED = 0X0002,

    EVENT_CODE_KEY_RELEASED =  0X0003,

    EVENT_CODE_BUTTON_PRESSED =  0X0004,

    EVENT_CODE_BUTTON_RELEASED = 0X0005,

    EVENT_CODE_MOUSE_MOVED =  0X0006,

    EVENT_CODE_MOUSE_WHEEL =  0X0007,

    EVENT_CODE_RESIZED =  0X0008,

    EVENT_CODE_DEBUG0 = 0X009,
    EVENT_CODE_DEBUG1 = 0X0011,
    EVENT_CODE_DEBUG2 = 0X0012,
    EVENT_CODE_DEBUG3 = 0X0013,
    
    MAX_EVENT_CODE = 0xFF

};
