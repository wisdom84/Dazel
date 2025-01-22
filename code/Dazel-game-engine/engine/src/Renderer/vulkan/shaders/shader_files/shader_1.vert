#version 450 // we are using glsl 4.5 version

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 in_textcord;
layout (location = 2) in vec3 normal;
layout(location = 3) in vec4 in_tangent;


layout(set=0, binding = 0, row_major) uniform global_uniform_object{
   mat4 projection;
   mat4 view;
   vec4 view_position;
   vec4 ambient_color;
   int mode;
}global_ubo;

layout(push_constant) uniform model_uniform_object{
  mat4 model;
}model_ubo;

layout (location = 0) flat out int out_mode;


layout (location = 1) out struct dto{
  vec2 text_cord;
  vec4 ambient;
  vec3 normal_dir;
  vec3 frag_position;
  vec3 view_position;
  vec4 tangent;
}out_dto;


void main(){
    mat3 m3_model = mat3(model_ubo.model);
    out_dto.text_cord = vec2 (in_textcord.x,  -in_textcord.y);
    out_dto.ambient = global_ubo.ambient_color;
    out_dto.normal_dir = m3_model*normal; 
    out_dto.frag_position = vec3( model_ubo.model * vec4(position, 1.0) );
    out_dto.view_position = vec3(global_ubo.view_position);
    out_dto.tangent = vec4(normalize(m3_model * in_tangent.xyz),in_tangent.w);
    out_mode = global_ubo.mode;
    gl_Position = global_ubo.projection * global_ubo.view * model_ubo.model*vec4(position,1.0);
}