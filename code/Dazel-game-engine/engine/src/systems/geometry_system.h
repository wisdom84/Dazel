#pragma once

#include "Renderer/renderer_types.inl"

typedef struct geometry_system_config{
  u32 max_geometry_count;
}geometry_system_config;

typedef struct geometry_config{
  u32 vertex_size;
  u32 vertex_count;
  u32 index_size;
  u32 index_count;
  void*indicies;
  void*verticies;
  vec3 min_extent;
  vec3 max_extent;
  vec3 center;
  char name[GEOMETRY_NAME_MAX_LENGTH];
  char material_name[MATERIAL_NAME_MAX_LENGTH];
  bool apply_material;
}geometry_config;

#define DEFAULT_GEOMETRY_NAME "default"

bool geometry_system_initialize(u64*memory_requirement, void*state, geometry_system_config config);

void geometry_system_shutdown(void*state);

geometry*geometry_system_acquire_by_id(u32 id);


geometry*geometry_system_acquire_from_config(geometry_config config, bool auto_release);

void geometry_release(geometry*geometry);

geometry*geometry_system_get_default();

geometry*geometry_system_get_default_2d();

geometry_config geometry_system_plane_config(float width, float height, u32 x_segment_count, u32 y_segment_count, float title_x, float title_y, const char*name, const char* material_name, bool apply_material);

geometry_config geometry_system_generate_cube_config(float width, float height, float depth, float tile_x, float tile_y, const char*name, const char*material_name, bool apply_material);

void geometry_system_config_dispose(geometry_config*config);

