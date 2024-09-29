#pragma once
#include "defines.h"
#include "Math/dlm.h"
#include "resources/resources.inl"
#include "Renderer_global_states.h"
using namespace dlm;

enum renderer_backend_type
{
  RENDERER_BACKEND_TYPE_VULKAN,
  RENDERER_BACKEND_TYPE_OPENGL,
  RENDERER_BACKEND_TYPE_DIRECT_X
};
enum builtin_renderpass
{
  BUILTIN_RENDERPASS_WORLD = 0x01,
  BUILTIN_RENDERPASS_UI = 0x02,
  BUILTIN_RENDERPASS_LIGHT = 0x03
};

enum shader_attribute_type
{
  SHADER_ATTRIBUTE_TYPE_FLOAT32,
  SHADER_ATTRIBUTE_TYPE_FLOAT32_2,
  SHADER_ATTRIBUTE_TYPE_FLOAT32_3,
  SHADER_ATTRIBUTE_TYPE_FLOAT32_4,
  SHADER_ATTRIBUTE_TYPE_INT8,
  SHADER_ATTRIBUTE_TYPE_INT8_2,
  SHADER_ATTRIBUTE_TYPE_INT8_3,
  SHADER_ATTRIBUTE_TYPE_INT8_4,
  SHADER_ATTRIBUTE_TYPE_UINT8,
  SHADER_ATTRIBUTE_TYPE_UINT8_2,
  SHADER_ATTRIBUTE_TYPE_UINT8_3,
  SHADER_ATTRIBUTE_TYPE_UINT8_4,
  SHADER_ATTRIBUTE_TYPE_INT16,
  SHADER_ATTRIBUTE_TYPE_INT16_2,
  SHADER_ATTRIBUTE_TYPE_INT16_3,
  SHADER_ATTRIBUTE_TYPE_INT16_4,
  SHADER_ATTRIBUTE_TYPE_UINT16,
  SHADER_ATTRIBUTE_TYPE_UINT16_2,
  SHADER_ATTRIBUTE_TYPE_UINT16_3,
  SHADER_ATTRIBUTE_TYPE_UINT16_4,
  SHADER_ATTRIBUTE_TYPE_INT32,
  SHADER_ATTRIBUTE_TYPE_INT32_2,
  SHADER_ATTRIBUTE_TYPE_INT32_3,
  SHADER_ATTRIBUTE_TYPE_INT32_4,
  SHADER_ATTRIBUTE_TYPE_UINT32,
  SHADER_ATTRIBUTE_TYPE_UINT32_2,
  SHADER_ATTRIBUTE_TYPE_UINT32_3,
  SHADER_ATTRIBUTE_TYPE_UINT32_4
};

enum shader_scope
{
  SHADER_SCOPE_LOCAL,
  SHADER_SCOPE_GLOBAL,
  SHADER_SCOPE_INSTANCE
};

typedef struct renderer_backend
{

  struct platform_state *plat_state;

  u64 frame_count;

  bool (*initialize)(struct renderer_backend *backend, const char *application_name, struct platform_state *plat_state, int width, int height);

  void (*shutdown)(struct renderer_backend *backend);

  void (*resized)(struct renderer_backend *backend, u16 width, u16 height);

  bool (*begin_frame)(struct renderer_backend *backend, float delta_time);

  void (*update_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, int mode, float delta_time, u32 shader_id);

  void (*update_light_global_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, int mode, float delta_time);

  void (*update_UI_global_state)(mat4 projection, mat4 view, int mode, float delta_time);

  bool (*begin_renderpass)(struct renderer_backend *backend, u8 renderpass_id);

  bool (*end_renderpass)(struct renderer_backend *backend, u8 renderpass_id);

  bool (*end_frame)(struct renderer_backend *backend, float delta_time);

  void (*create_texture)(const char *name, int width, int height, int channel_count, const u8 *pixels, bool has_transparency, struct Texture *out_texture);

  void (*destroy_texture)(struct Texture *texture);

  bool (*create_geometry)(geometry *geo_obj, u32 vertex_count, u32 vertex_size, const void *verticies, u32 index_size, u32 index_count, const void *indicies);

  void (*destroy_geometry)(geometry *geometry);

  void (*draw_geometry)(geometry_render_data data);

  bool (*shader_create)(struct material_shader*shader);

  bool (*acquire_shader_resources)(struct material_shader*shader, u32*material_id);

  bool (*release_shader_resources)(struct material_shader*shader, u32 material_id);

// bool (*shader_destroy)(vulkan_shader *shader);

// bool (*shader_add_attributes)(vulkan_shader *shader, const char *name, shader_attribute_type type);

// bool (*shader_add_sampler)(vulkan_shader *shader, const char *sampler_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_i16)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_i32)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_u8)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_u16)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_u32)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_f32)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_vec2)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_vec3)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_vec4)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_add_uniform_mat4)(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

// bool (*shader_intialize)(vulkan_shader *shader);

// bool (*shader_use)(vulkan_shader *shader);

// bool (*shader_bind_globals)(vulkan_shader *shader);

// bool (*shader_bind_instance)(vulkan_shader *shader, u32 instance_id);

// bool (*shader_apply_globals)(vulkan_shader *shader);

// bool (*shader_apply_instance)(vulkan_shader *shader);

// bool (*shader_acquire_instance_resources)(vulkan_shader *shader, u32 *out_instance_id);

// bool (*shader_release_instance_resources)(vulkan_shader *shader, u32 out_instance_id);

// u32 (*shader_uniform_location)(vulkan_shader *shader, const char *uniform_name);

// bool (*shader_check_uniform_size)(vulkan_shader *shader, u32 location, u32 expected_size);

// bool (*shader_set_sampler)(vulkan_shader *shader, u32 location, Texture *t);

// bool (*form(vulkan_shader *shader, u32 location, void *value, u32 size);

// bool (*shader_set_uniform_i16)(vulkan_shader *shader, u32 location, i16 value);

// bool (*shader_set_uniform_i32)(vulkan_shader *shader, u32 location, int value);

// bool (*shader_set_uniform_u8)(vulkan_shader *shader, u32 location, u8 value);

// bool (*shader_set_uniform_u16)(vulkan_shader *shader, u32 location, u16 value);

// bool (*shader_set_uniform_u32)(vulkan_shader *shader, u32 location, u32 value);

// bool (*shader_set_uniform_f32)(vulkan_shader *shader, u32 location, float value);

// bool (*shader_set_uniform_vec2)(vulkan_shader *shader, u32 location, vec2 value);

// bool (*shader_set_uniform_vec2f)(vulkan_shader *shader, u32 location, float value_0, float value_1);

// bool (*shader_set_uniform_vec3)(vulkan_shader *shader, u32 location, vec3 value);

// bool (*shader_set_uniform_vec3f)(vulkan_shader *shader, u32 location, float value_0, float value_1, float value_2);

// bool (*shader_set_uniform_vec4)(vulkan_shader *shader, u32 location, vec4 value);

// bool (*shader_set_uniform_vec4f)(vulkan_shader *shader, u32 location, float value_0, float value_1, float value_2, float value_3);

// bool (*shader_set_uniform_mat4)(vulkan_shader *shader, u32 location, mat4 value);

} renderer_backend;

typedef struct render_packet
{
  float delta_time;
  u32 geometry_count;
  u32 UI_geometry_count;
  geometry_render_data *geometries;
  geometry_render_data *UI_geometries;
} render_packet;