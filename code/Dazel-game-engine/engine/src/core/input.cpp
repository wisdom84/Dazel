#include "input.h"
#include "event_manager.h"
#include "Dmemory.h"
#include "logger.h"


typedef struct keyboard_state{
    bool keys[256];
}keyboard_state;

typedef struct mouse_state{
    int x;
    int y;
    bool mouse_button[BUTTON_MAX_BUTTON];
}mouse_state;

typedef struct input_state{
    keyboard_state keybord_previous_state;
    keyboard_state keybord_current_state;
    mouse_state mouse_previous_state;
    mouse_state mouse_current_state;
}input_state;
static bool intialized =false;
static input_state state;

void input_intialize(){
  if(intialized){
    intialized=false;
    return;
  }
  intialized=true;
};

void input_shutdown(){
    intialized=false;
};

void input_update(float delta_time){
    if(!intialized){
        return;
    }
    // copy the data of the current state to the previous state
    Dmemory_copy(&state.keybord_previous_state, &state.keybord_current_state,sizeof(keyboard_state));
    Dmemory_copy(&state.mouse_previous_state, &state.mouse_current_state, sizeof(mouse_state));
}

void input_process_key(keys key, bool pressed){
    if(state.keybord_current_state.keys[key] != pressed){
        // update the state of the key
         state.keybord_current_state.keys[key] = pressed;
        // fire off an event for immediate processing
            event_context context;
            context.u16_bytes[0] = key;
            event_fire(pressed?EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0,context);
    }
}

void input_process_button(mouse_button button, bool pressed){
       if(state.mouse_current_state.mouse_button[button] != pressed){
        // update the state of the key
         state.mouse_current_state.mouse_button[button] = pressed;
        // fire off an event for immediate processing
            event_context context;
            context.u16_bytes[0] = button;
            event_fire(pressed?EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED,0,context);
    }
};
void input_process_mouse_move(int x, int y){
        if(state.mouse_current_state.x !=x || state.mouse_current_state.y != y){
        // update the state of the position
           state.mouse_current_state.x =x;
           state.mouse_current_state.y =y;
        // fire off an event for immediate processing
            event_context context;
            context.u16_bytes[0] = x;
            context.u16_bytes[1] = y;
            event_fire(EVENT_CODE_MOUSE_MOVED,0,context);
        }
};
void input_process_mouse_wheel(int z_delta){
   event_context context;
   context.u16_bytes[0] = z_delta;
   event_fire(EVENT_CODE_MOUSE_WHEEL, 0,context);
};

bool input_key_is_down(keys key){
   if(!intialized){
    return false;
   }
   return state.keybord_current_state.keys[key] == true;
};
bool input_key_is_up(keys key){
  if(!intialized){
    return true;
  }
   return state.keybord_current_state.keys[key] == false;
};
bool input_key_was_down(keys key){
 if(!intialized){
    return false;
  }
   return state.keybord_previous_state.keys[key] == true;

};
bool input_key_was_up(keys key){
   if(!intialized){
    return true;
  }
   return state.keybord_previous_state.keys[key] == false;
};
bool input_is_button_down(mouse_button button){
   if(!intialized){
    return false;
  }
   return state.mouse_current_state.mouse_button[button] == true;
};
bool input_is_button_up(mouse_button button){
 if(!intialized){
    return true;
  }
   return state.mouse_current_state.mouse_button[button] == false;   
};
bool input_was_button_down(mouse_button button){
if(!intialized){
    return true;
  }
   return state.mouse_current_state.mouse_button[button] == true;
};
bool input_was_button_up(mouse_button button){
 if(!intialized){
    return false;
  }
   return state.mouse_current_state.mouse_button[button] == false;
};
void input_get_mouse_position(int*x,int*y){
    if(!intialized){
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_current_state.x;
    *y = state.mouse_current_state.y;
};
void input_get_previous_mouse_position(int*x,int*y){
     if(!intialized){
        *x = 0;
        *y = 0;
        return;
    }
    *x = state.mouse_previous_state.x;
    *y = state.mouse_previous_state.y;
};