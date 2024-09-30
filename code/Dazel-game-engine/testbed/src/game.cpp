#include "game.h"

#include <core/input.h> // Hack method 
#include <core/logger.h>
#include <core/Dmemory.h>
#include <Renderer/renderer_frontend.h>
float Clamp(float value, float min_num, float max_num){
  return value <= min_num ? min_num : value >= max_num ? max_num : value;
}
void recalculate_camera_view_matrix(game_state*state){
   if( state->camera_view_dirty){
    mat4 rotation = mat4_euler_xyz(state->camera_euler.x,state->camera_euler.y,state->camera_euler.z);
    mat4 translation = mat4_homogeneous_translation(state->camera_position.x,state->camera_position.y,state->camera_position.z);
    state->view =translation*rotation;
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty= false;
   }
}
void camera_yaw(game_state*state,float amount){
   state->camera_euler.y += amount;
   state->camera_view_dirty = true; 
}

void camera_pitch(game_state*state,float amount){
   state->camera_euler.x += amount;
   float limit = 89.0f;
   state->camera_euler.x = Clamp(state->camera_euler.x,-limit,limit);
   state->camera_view_dirty = true; 
}

 bool game_initialize(struct game*game_inst){
    DDEBUG("game initialization called");
    game_state*state = (game_state*)game_inst->state;
    state->camera_position = vec3_create(0.0f,0.0f,25.0f);
    state->camera_euler = vec3_zero();
    state->view = mat4_homogeneous_translation(0.0f,0.0f,25.0f);
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = true;
   return true;
 };


// float movespeed = 1.0f;
// vec4 velocity = vec4_zero();
 bool game_update(struct game*game_inst, float delta_time){
   game_state*state =(game_state*)game_inst->state;
    static u64 alloc_count = 0;
    alloc_count = get_memory_alloc_count();
    if(input_key_is_down(KEY_M)&& input_key_was_down(KEY_M)){
      DDEBUG("Allocations: %i", alloc_count);
    }
    if(input_key_is_down(KEY_A) || input_key_is_down(KEY_LEFT_ARROW)){
      camera_yaw(state,50.0f*delta_time);
    }
    if(input_key_is_down(KEY_D) || input_key_is_down(KEY_RIGHT_ARROW)){
      camera_yaw(state,-50.0f*delta_time);
    }
    if(input_key_is_down(KEY_UP_ARROW)){
      camera_pitch(state,-50.0f*delta_time);
    }
    if(input_key_is_down(KEY_DOWN_ARROW)){
      camera_pitch(state,+50.0f*delta_time);
    }
    float temp_move_speed = 10.0f;
    vec3 velocity =vec3_zero();
    if(input_key_is_down(KEY_W)){ // zooming
      vec3 forward = mat_4_forward_vec(state->view);
      velocity = Add_vec3(velocity,forward);
    }
    if(input_key_is_down(KEY_S)){ // zooming 
      vec3 backward = mat_4_backward_vec(state->view);
      velocity = Add_vec3(velocity,backward);
    }

     if(input_key_is_down(KEY_Q)){ // for straving
      vec3 left = mat_4_left_vec(state->view);
      velocity = Add_vec3(velocity,left);
    }
    if(input_key_is_down(KEY_E)){ // straving
      vec3 right = mat_4_right_vec(state->view);
      velocity = Add_vec3(velocity,right);
    }
    if(input_key_is_down(KEY_SPACEBAR)){
      velocity.y += 1.0f;
    }
    if(input_key_is_down(KEY_X)){
      velocity.y -= 1.0f;
    }
    vec3 z = vec3_zero();
    if(!vec3_compare(z,velocity,0.0002f)){
      velocity = vec3_nomalize(velocity);
      state->camera_position.x += velocity.x * temp_move_speed * delta_time;
      state->camera_position.y += velocity.y * temp_move_speed * delta_time;
      state->camera_position.z += velocity.z * temp_move_speed * delta_time;
      state->camera_view_dirty = true;
    }
    recalculate_camera_view_matrix(state);
    renderer_set_view(state->view,  state->camera_position);
    return true;
 };

 bool game_render(struct game* game_inst, float delta_time){
   return true;
 };
 void game_onresize(struct game* game_inst, int width, int height){
     //TODO: do something
 }