#version 450 
layout (location = 0) out vec4 outcolor;
// layout (location = 0) in int out_mode;

layout (set = 1,binding = 0) uniform local_uniform_object{
  vec4 diffuse_color;
}object_ubo;

layout (set = 1, binding = 1) uniform sampler2D texture_sampler;

layout (location = 1) in struct dto {
   vec2 text_cord;
}in_dto;

void main(){
  outcolor = object_ubo.diffuse_color * texture(texture_sampler,in_dto.text_cord);
   
}