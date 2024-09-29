#version 450 // we are using glsl 4.5 version

layout(location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout(location = 2) in vec2 in_textcord;


layout(set=0, binding = 0, row_major) uniform global_uniform_object{
   mat4 projection;
   mat4 view;
   vec4 ambient_color;
   vec3 view_position;
}global_ubo;

layout(push_constant) uniform model_uniform_object{
  mat4 model;
}model_ubo;

// layout (location = 0) out int out_mode;


layout (location = 1) out struct dto{
  vec2 text_cord;
  vec4 ambient;
  vec3 normal_dir;
  vec3 frag_position;
  vec3 view_position;
}out_dto;

void main(){
    out_dto.text_cord = vec2 (in_textcord.x,  1.0-in_textcord.y);
    out_dto.ambient = global_ubo.ambient_color;
    out_dto.normal_dir = mat3(model_ubo.model) * normal; 
    out_dto.frag_position = vec3( model_ubo.model * vec4(position, 1.0) );
    out_dto.view_position = global_ubo.view_position;
    gl_Position = global_ubo.projection * global_ubo.view * model_ubo.model*vec4(position,1.0);
}