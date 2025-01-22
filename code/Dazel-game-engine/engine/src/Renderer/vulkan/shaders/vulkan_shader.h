#pragma once
#include "Renderer/vulkan/vulkan_types.inl"
#include "containers/hashtable.h"
using namespace dlm;
struct Texture;
// #define VULKAN_SHADER_MAX_PUSH_CONST_RANGES  2
// #define VULKAN_SHADER_MAX_STAGE 2
// #define VULKAN_SHADER_MAX_GLOBAL_TEXTURE 2
// #define VULKAN_SHADER_MAX_ATTRIBUTES 2
// #define VULKAN_SHADER_MAX_UNIFORM 2
// #define VULKAN_SHADER_MAX_INSTANCE_TEXTURE 1024
// #define VULKAN_SHADER_MAX_BINDINGS 2

// enum vulkan_shader_state {
//   VULKAN_SHADER_STATE_NOT_CREATED,

//   VULKAN_SHADER_STATE_UNINITIALIZED,

//    VULKAN_SHADER_STATE_INITIALIZED,
// };

// enum shader_scope{
//   SHADER_SCOPE_LOCAL,
//   SHADER_SCOPE_GLOBAL,
//   SHADER_SCOPE_INSTANCE
// };

// enum shader_attribute_type{

//   SHADER_ATTRIBUTE_TYPE_FLOAT32,
//   SHADER_ATTRIBUTE_TYPE_FLOAT32_2,
//   SHADER_ATTRIBUTE_TYPE_FLOAT32_3,
//   SHADER_ATTRIBUTE_TYPE_FLOAT32_4,
//   SHADER_ATTRIBUTE_TYPE_INT8,
//   SHADER_ATTRIBUTE_TYPE_INT8_2,
//   SHADER_ATTRIBUTE_TYPE_INT8_3,
//   SHADER_ATTRIBUTE_TYPE_INT8_4,
//   SHADER_ATTRIBUTE_TYPE_UINT8,
//   SHADER_ATTRIBUTE_TYPE_UINT8_2,
//   SHADER_ATTRIBUTE_TYPE_UINT8_3,
//   SHADER_ATTRIBUTE_TYPE_UINT8_4,
//   SHADER_ATTRIBUTE_TYPE_INT16,
//   SHADER_ATTRIBUTE_TYPE_INT16_2,
//   SHADER_ATTRIBUTE_TYPE_INT16_3,
//   SHADER_ATTRIBUTE_TYPE_INT16_4,
//   SHADER_ATTRIBUTE_TYPE_UINT16,
//   SHADER_ATTRIBUTE_TYPE_UINT16_2,
//   SHADER_ATTRIBUTE_TYPE_UINT16_3,
//   SHADER_ATTRIBUTE_TYPE_UINT16_4,
//   SHADER_ATTRIBUTE_TYPE_INT32,
//   SHADER_ATTRIBUTE_TYPE_INT32_2,
//   SHADER_ATTRIBUTE_TYPE_INT32_3,
//   SHADER_ATTRIBUTE_TYPE_INT32_4,
//   SHADER_ATTRIBUTE_TYPE_UINT32,
//   SHADER_ATTRIBUTE_TYPE_UINT32_2,
//   SHADER_ATTRIBUTE_TYPE_UINT32_3,
//   SHADER_ATTRIBUTE_TYPE_UINT32_4
// };

// typedef struct vulkan_shader_stage_config{
//   char stage_str[7];
//   VkShaderStageFlagBits stage_flag;
// }vulkan_shader_stage_config;

// typedef struct vulkan_shader_stage_new{
//   VkPipelineShaderStageCreateInfo shader_satge_create_info;
//   VkShaderModule handle;
//   VkShaderModuleCreateInfo shader_module_create_info;
// }vulkan_shader_stage_new;

// typedef struct vulkan_descriptor_set_config{
//   VkDescriptorSetLayoutBinding binding[2];
//   u64 binding_count;
// }vulkan_descriptor_set_config;

// typedef struct   vulkan_descriptor_state_new{
//    u32 generations[3];
//     u32 id[3];
// }vulkan_descriptor_state_new;

// typedef struct     vulkan_descriptor_set_state{
//     VkDescriptorSet descriptor_set[3];
//     vulkan_descriptor_state_new descriptor_state[2];
// }vulkan_descriptor_set_state;

// typedef struct vulkan_uniform_lookup_entry{
//   shader_scope scope;
//   u64 location;
//   u64 size;
//   u64 offset;
//   u32 index;
//   u32 set_index;
// } vulkan_uniform_lookup_entry;

// typedef struct range{
// //    mat4 push_range;
//    u64 offset;
//    u64 size;
// }range;

// typedef struct shader_config{
//  u64 attribute_stride;
//  u64 push_constant_range_count;
//  range push_constant_range[VULKAN_SHADER_MAX_PUSH_CONST_RANGES];
//  u64 max_desrciptor_set_count;
//  vulkan_shader_stage_config stages[VULKAN_SHADER_MAX_STAGE];
//  u64 descriptor_set_count;
//  vulkan_descriptor_set_config descriptor_sets[2];
//  VkDescriptorPoolSize pool_size[2];
//  u64 stage_count;
//  VkVertexInputAttributeDescription attributes[VULKAN_SHADER_MAX_ATTRIBUTES];
//  u64 attribute_count;
// }shader_config;

// typedef struct   vulkan_shader_instance_state{
//   u64 id;
//   u64 offset;
//   Texture*instance_texture[2];
//   vulkan_descriptor_set_state descriptor_set_state;
// }  vulkan_shader_instance_state;

// typedef struct vulkan_shader{
//    vulkan_context*context;
//    vulkan_shader_state state;
//    const char*name;
//    bool use_instance;
//    bool use_push_constants;
//    range push_constant_range[VULKAN_SHADER_MAX_PUSH_CONST_RANGES];
//    VkDescriptorSetLayout descriptor_set_layout[3];
//    VkDescriptorPool descriptor_pool;
//    VkDescriptorSet global_descriptor_set[3];
//    vulkan_renderpass*renderpass;
//    shader_config config;
//    vulkan_shader_stage_new stages[VULKAN_SHADER_MAX_STAGE];
//   vulkan_shader_instance_state instance_state[1024];
//    u64 bound_instance_id;
//    Texture*global_texture[VULKAN_SHADER_MAX_GLOBAL_TEXTURE];
//    u32 global_texture_count;
//    u32 instance_texture_count;
//    vulkan_buffer uniform_buffer;
//    void*mapped_uniform_buffer_block;
//    vulkan_uniform_lookup_entry uniforms[VULKAN_SHADER_MAX_UNIFORM];
//    u64 uniform_count;
//    hashtable uniform_look_up;
//    void*hash_table_block;
//    u64 global_ubo_size;
//    u64 global_ubo_stride;
//    u64 global_ubo_offset;
//    u64 bound_ubo_offset;
//    u64 ubo_size;
//    u64 ubo_stride;
//    u64 required_ubo_alignment;
//    u64 push_constant_stride;
//    u64 push_constant_size;
//    vulkan_pipeline pipeline;
// }vulkan_shader;

bool vulkan_shader_create(vulkan_context *context, const char *name, vulkan_renderpass *renderpass, VkShaderStageFlags stages, const char*vertex_stage, const char*fragment_stage, u32 max_descriptor_set_count, bool use_instance, bool use_local, vulkan_shader *out_shader);

bool vulkan_shader_destroy(vulkan_shader *shader);

bool vulkan_shader_add_attributes(vulkan_shader *shader, const char *name, shader_attribute_type type);

bool vulkan_shader_add_sampler(vulkan_shader *shader, const char *sampler_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_i16(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_i32(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_u8(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_u16(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_u32(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_f32(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_vec2(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_vec3(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_vec4(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_add_uniform_mat4(vulkan_shader *shader, const char *uniform_name, shader_scope scope, u32 *out_location);

bool vulkan_shader_intialize(vulkan_shader *shader);

bool vulkan_shader_use(vulkan_shader *shader);

bool vulkan_shader_bind_globals(vulkan_shader *shader);

bool vulkan_shader_bind_instance(vulkan_shader *shader, u32 instance_id);

bool vulkan_shader_apply_globals(vulkan_shader *shader);

bool vulkan_shader_apply_instance(vulkan_shader *shader);

bool vulkan_shader_acquire_instance_resources(vulkan_shader *shader, u32 *out_instance_id);

bool vulkan_shader_release_instance_resources(vulkan_shader *shader, u32 out_instance_id);

u32 vulkan_shader_uniform_location(vulkan_shader *shader, const char *uniform_name);

u32 vulkan_shader_get_uniform_size(vulkan_shader*shader, u32 location);

bool check_uniform_size(vulkan_shader *shader, u32 location, u32 expected_size);

bool vulkan_shader_set_sampler(vulkan_shader *shader, u32 location, Texture*t);

bool set_uniform(vulkan_shader *shader, u32 location, void *value, u32 size);

bool vulkan_shader_set_uniform_i16(vulkan_shader *shader, u32 location, i16 value);

bool vulkan_shader_set_uniform_i32(vulkan_shader *shader, u32 location, void*value);

bool vulkan_shader_set_uniform_u8(vulkan_shader *shader, u32 location, u8 value);

bool vulkan_shader_set_uniform_u16(vulkan_shader *shader, u32 location, u16 value);

bool vulkan_shader_set_uniform_u32(vulkan_shader *shader, u32 location, u32 value);

bool vulkan_shader_set_uniform_f32(vulkan_shader *shader, u32 location, void*value);

bool vulkan_shader_set_uniform_vec2(vulkan_shader *shader, u32 location, void*value);

bool vulkan_shader_set_uniform_vec2f(vulkan_shader *shader, u32 location, float value_0, float value_1);

bool vulkan_shader_set_uniform_vec3(vulkan_shader *shader, u32 location, void*value);

bool vulkan_shader_set_uniform_vec3f(vulkan_shader *shader, u32 location, float value_0, float value_1, float value_2);

bool vulkan_shader_set_uniform_vec4(vulkan_shader *shader, u32 location, void*value);

bool vulkan_shader_set_uniform_vec4f(vulkan_shader *shader, u32 location, float value_0, float value_1, float value_2, float value_3);

bool vulkan_shader_set_uniform_mat4(vulkan_shader *shader, u32 location, void*value);
