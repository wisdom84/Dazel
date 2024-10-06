#pragma once
// #include "renderer_backend.h"
// #include "vulkan_types.inl"
#include "defines.h"
#include "Math/dlm.h"
#include "Renderer/Renderer_global_states.h"
using namespace dlm;
// #define STB_IMAGE_IMPLEMENTATION
bool vulkan_backend_initialize(struct renderer_backend* backend, const char* application_name, struct platform_state* plat_state, int width, int height);

void vulkan_backend_shutdown(struct renderer_backend* backend);

void vulkan_backend_resized(struct renderer_backend* backend, u16 width, u16 height);

bool vulkan_backend_begin_frame(struct renderer_backend* backend, float delta_time);

void vulkan_renderer_update_global_state(mat4 projection,mat4 view,vec3 view_position,vec4 ambient_color,int mode, float delta_time, global_uniform_object*object, u32 shader_id);
void vulkan_renderer_update_light_global_state(mat4 projection,mat4 view,vec3 view_position,vec4 ambient_color,int mode,float delta_time);
void vulkan_renderer_update_UI_global_state(mat4 projection,mat4 view,int mode,float delta_time);

void vulkan_create_texture( const char*name,int width, int height, int channel_count, const u8*pixels,bool has_transparency,struct Texture*out_texture);

void  vulkan_destroy_texture(struct Texture*texture);

bool vulkan_backend_end_frame(struct renderer_backend* backend, float delta_time);



void vulkan_renderer_draw_geometry(geometry_render_data data, u32 value);
bool vulkan_renderer_begin_renderpass(struct renderer_backend*backend, u8 renderpass_id);
bool vulkan_renderer_end_renderpass(struct renderer_backend*backend, u8 renderpass_id);

bool vulkan_create_geometry(geometry*geo_obj,u32 vertex_count,u32 vertex_size , const void*verticies, u32 index_size,u32 index_count, const void*indicies);

void vulkan_destroy_geometry(geometry*geometry);

bool vulkan_create_material_shader(struct material_shader*shader);

bool vulkan_acquire_material_shader_resources(struct material_shader*shader,u32*material_id);
bool vulkan_release_material_shader_resources(struct material_shader*shader,u32 material_id);


