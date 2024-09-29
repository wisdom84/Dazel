#pragma once
#include <defines.h>
#include <core/logger.h>
#include <game_types.h>

typedef struct game_state{
   float delta_time;
   mat4 view;
   vec3 camera_position;
   vec3 camera_euler;
   bool camera_view_dirty;
}game_state;

 bool game_initialize(struct game*game_inst);

 bool game_update(struct game* game_inst, float delta_time);

 bool game_render(struct game* game_inst, float delta_time);

 void game_onresize(struct game* game_inst, int width, int height);