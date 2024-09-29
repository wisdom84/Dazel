#version 450 // we are using glsl 4.5 version

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 in_textcord;


layout(set=0, binding = 0, row_major) uniform global_uniform_object{
   mat4 projection;
   mat4 view;
}global_ubo;

layout(push_constant) uniform model_uniform_object{
  mat4 model;
}model_ubo;

// layout (location = 0) out int out_mode;


layout (location = 1) out struct dto{
  vec2 text_cord;
}out_dto;

void main(){
    out_dto.text_cord = vec2(in_textcord.x,in_textcord.y);
    gl_Position = global_ubo.projection * global_ubo.view * model_ubo.model*vec4(position,0.0,1.0);
}