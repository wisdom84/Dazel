#include "clock.h"
#include "platform/platform.h"
#include "logger.h"

void clock_update(clock_time*clock){
   if(clock->start_time != 0){
    clock->elapsed_time = get_absolute_time()-clock->start_time;// elapsed time
    // current time (from  the function get absolute time ) minus the start time 
   }
};
void clock_start(clock_time*clock){
    clock->start_time = get_absolute_time();
    clock->elapsed_time =0;
    // when ever this function is invoked the start time is set to the current time while the 
    // elasped time is set to 0
};
void clock_stop(clock_time*clock){
  clock->start_time= 0; // sets the start time to 0
};