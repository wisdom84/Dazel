#pragma once 
#include "defines.h"
#include "renderer_types.inl"


struct static_mesh_data;

struct platform_state;

bool renderer_initialize(const char*application_name, struct platform_state*plat_state, int width, int height);

void renderer_shutdown();

void renderer_on_resized(u16 width, u16 height);

bool renderer_draw_frame(render_packet*packet);

EXP void renderer_set_view(mat4 view, vec3 camera_position);


void renderer_create_texture( const char*name,int width, int height,int channel_count, const u8*pixels, bool has_transparency, struct Texture*out_texture);

void renderer_destroy_texture(struct Texture*texture);

bool renderer_create_material_shader(struct material_shader*shader);

bool renderer_acquire_shader_resources(struct material_shader*shader, u32*material_id);

bool renderer_release_shader_resources(struct material_shader*shader, u32 material_id);

bool renderer_create_geometry(geometry*geo_obj,u32 vertex_count,u32 vertex_size , const void*verticies, u32 index_size,u32 index_count, const void*indicies);

void renderer_destroy_geometry(geometry*geometry);

void draw_geometry(geometry_render_data data);

void renderer_activate_shader(u32 id, bool is_active);