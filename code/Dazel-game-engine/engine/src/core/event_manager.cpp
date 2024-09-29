#include "event_manager.h"
#include "Dmemory.h"
#include "core/logger.h"
#define MAX_MESSAGE_CODES 16384
typedef struct registered_event{
   void*listener;
   PFN_on_event call_back;
}registered_event;

 typedef struct event_code_entry{
   registered_event*events;
}event_code_entry;

typedef struct event_system_state{
    event_code_entry registered[MAX_MESSAGE_CODES];
}event_system_state;

static bool is_intialized= false;
static event_system_state state;

bool event_intialize(){
   if(is_intialized){
      is_intialized=false;
      return false;
   }
   is_intialized=true;
   return true;
}
void event_shutdown(){
   // deallocting all the dynamic arrays used to  store  the events
   for(u64 i=0; i < MAX_EVENT_CODE; i++){
      if(state.registered[i].events !=0){
         darray_destroy_array(state.registered[i].events);
         state.registered[i].events = 0;
      }
   }
    is_intialized=false;
};

 bool event_register(u16 code, void*listener, PFN_on_event on_event){ 
   if(!is_intialized){
      DWARNING("tried to register event after event_system_shutdown or before event_system_intialization");
      return false;
   }
   //we check if the array of registered events are empty
   if(state.registered[code].events ==0){
         state.registered[code].events = (registered_event*)darray_create(registered_event);
   }
   // checking if a event has already been registered for that listener
   u64 registered_event_count = darray_length(state.registered[code].events);
   for(auto i=0; i<registered_event_count; i++){
      if(state.registered[code].events[i].listener == listener){
         return false;
      }
   }
   // register the listener
   registered_event event;
   event.listener=listener;
   event.call_back = on_event;
   darray_push(state.registered[code].events, event, registered_event);
   return true;
 }
 bool event_unregister(u16 code, void*listener, PFN_on_event on_event){
     if(!is_intialized){
        DWARNING("tried to unregister event after event_system_shutdown or before event intialization");
        return false;
   }
  //we check if the array of registered events are empty
  if(darray_length(state.registered[code].events) == 0){
     return false;
  }
  // check if the listener is unregistered or not
    u64 registered_event_count = darray_length(state.registered[code].events);
   for(auto i=0; i<registered_event_count; i++){
      registered_event e=state.registered[code].events[i];
      if(e.listener == listener && e.call_back == on_event){
         // the listener and callback function has to mach for us to unregister the event
          registered_event popped;
          // we pop at the index value
         darray_pop_at(state.registered[code].events,i,&popped);
         return true;
      }
   }
   return false;
 };
bool event_fire(u16 code, void*sender, event_context data){
   if(!is_intialized){
      return false;
   }
   if(state.registered[code].events ==0){
       return false;
   }
   u64 registered_event_count = darray_length(state.registered[code].events);
   for(auto i=0; i < registered_event_count; i++){
      registered_event e=state.registered[code].events[i];
     if( e.call_back(code,sender, e.listener, data)){// we are calling the first ever listener
     // to handle the message once we call the event fire function
              return true;
     };

   }
   return false;
};

