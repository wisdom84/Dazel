#include "defines.h"

typedef struct clock_time{
   double start_time;
   double elapsed_time;
}clock_time;

void clock_update(clock_time*clock);
void clock_start(clock_time*clock);
void clock_stop(clock_time*clock);