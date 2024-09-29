#pragma once 
#include "defines.h"
typedef struct application_config {
   // window starting x-axis position
   int x;
   // window starting y-axis position
   int y;
   // window height
   int height;
   // window width
   int width;
   // the application name 
  const char * application_name;
}application_config;
EXP bool application_create(struct game* game_inst);

EXP bool application_run();
void application_get_framebuffer(u64*width, u64*height);
