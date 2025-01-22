#pragma once 
#include "Math/dlm.h"
#include "resources/resources.inl"
using namespace dlm;
typedef struct global_uniform_object{
    mat4 projection;
    mat4 view;
    vec4 camera_position;
    vec4 ambient_color;
    u32  render_mode;
}global_uniform_object;

typedef struct uniform_object{
  mat4 model; 
  vec4 diffuse_color;
  float shineness;
  int sample_count_less;
}uniform_object;

typedef struct sampler_uniform_object{

   Texture*diffuse_texture;
   Texture*specular_texture;
}sampler_uniform_object;




typedef struct global_UI_uniform_object{
  mat4 projection;
  mat4 view;
  mat4 m_reserved_0;
  mat4 m_reserved_1;
}global_UI_uniform_object;

typedef struct uniform_UI_object{
  vec4 diffuse_color;
  vec4 v_reserved_0;
  vec4 v_reserved_1;
  vec4 v_reserved_2;
}uniform_UI_object;



typedef struct geometry_render_data{
 mat4 model;
 geometry*geo_obj;
}geometry_render_data;