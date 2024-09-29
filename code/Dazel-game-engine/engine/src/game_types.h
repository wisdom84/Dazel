#pragma once
#include <core/application.h>
#include "Math/dlm.h"
using namespace dlm;


// basic game state or the game structure when our application is called to 
// create it
typedef struct game {
  // the app config
  application_config app_config;
  
  bool (*initialize)(struct game*game_inst);

  bool (*update)(struct game* game_inst, float delta_time);

  bool (*render)(struct game* game_inst, float delta_time);

  void (*on_resize)(struct game* game_inst, int width, int height);

  void*state;

}game;


