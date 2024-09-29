#pragma once
#include <core/asserts.h>
#include "defines.h"
#include <vulkan/vulkan.h>
#include "Renderer/Renderer_global_states.h"
#include "resources/resources.inl"
#include "containers/free_list.h"
#include "containers/hashtable.h"
#include "Renderer/renderer_types.inl"
#define VK_CHECK(expr)           \
  {                              \
    DASSERT(expr == VK_SUCCESS); \
  }
typedef struct object_uniform_object
{
  mat4 object;
} object_uniform_object;

typedef struct str_to_enum{
  shader_attribute_type value;
  char str_variant[128];
}str_to_enum;

typedef struct vulkan_buffer
{
  VkBuffer handle;
  VkDeviceMemory memory;
  VkDeviceSize size;
  VkMemoryPropertyFlags memory_flags;
  bool is_locked;
  VkBufferUsageFlags buffer_usage;
  int memory_index;
  u64 freelist_memory_requirement;
  void *freelist_block;
  freelist buffer_freelist;
} vulkan_buffer;

typedef struct vulkan_swapchain_support_info
{
  VkSurfaceCapabilitiesKHR capabilities;
  u32 format_count;
  VkSurfaceFormatKHR *surface_format;
  u32 present_mode_count;
  VkPresentModeKHR *present_mode;
} vulkan_swapchain_support_info;

typedef struct vulkan_device
{
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  VkPhysicalDeviceProperties properties;

  VkPhysicalDeviceFeatures features;
  VkPhysicalDeviceMemoryProperties memory;
  vulkan_swapchain_support_info swapchain_support;

  VkQueue graphics;
  VkQueue present;
  VkQueue transfer;

  u32 graphics_queue_family_index;
  u32 present_queue_family_index;
  u32 compute_queue_family_index;
  u32 transfer_queue_family_index;
  VkFormat depth_format;
  VkCommandPool graphics_command_pool;

  bool supports_device_local_host_visible;
} vulkan_device;

typedef struct vulkan_image
{
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u64 width;
  u64 height;
} vulkan_image;
enum vulkan_command_buffer_state
{
  COMMAND_BUFFER_STATE_READY,
  COMMAND_BUFFER_STATE_RECORDING,
  COMMAND_BUFFER_STATE_IN_RENDER_PASS,
  COMMAND_BUFFER_STATE_RECORDING_ENDED,
  COMMAND_BUFFER_STATE_SUBMITTED,
  COMMAND_BUFFER_STATE_NOT_ALLOCATED
};

typedef struct vulkan_command_buffer
{
  VkCommandBuffer handle;
  vulkan_command_buffer_state state;
} vulkan_command_buffer;

enum vulkan_render_pass_state
{
  READY,
  RECORDING,
  IN_RENDER_PASS,
  RECORDING_ENDED,
  SUBMITTED,
  NOT_ALLOCATED
};

typedef struct vulkan_renderpass
{
  VkRenderPass handle;
  float x, y, w, h;
  float r, g, b, a;
  float depth;
  u64 stencil;
  u8 clear_flag;
  bool has_prev_pass;
  bool has_next_pass;
  vulkan_render_pass_state state;
} vulkan_renderpass;

typedef struct vulkan_frame_buffers
{
  VkFramebuffer handle;
  u64 attachment_count;
  VkImageView *attachment;
  vulkan_renderpass *renderpass;
} vulkan_frame_buffers;

typedef struct vulkan_swapchain
{
  VkSurfaceFormatKHR image_format;
  VkPresentModeKHR present_mode;
  VkSwapchainKHR handle;
  VkExtent2D extent;
  u64 max_frames_in_flight;
  uint32_t image_count;
  VkImage *images;
  VkImageView *image_views;
  vulkan_image depth_attachment;
  vulkan_frame_buffers *frame_buffers;
} vulkan_swapchain;

typedef struct vulkan_fence
{
  VkFence handle;
  bool is_signaled;
} vulkan_fence;

typedef struct vulkan_pipeline
{
  VkPipeline handle;
  VkPipelineLayout layout_handle;
} vulkan_pipeline;

typedef struct vulkan_shader_stage
{
  VkShaderModuleCreateInfo create_info;
  VkShaderModule handle;
  VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info;
} vulkan_shader_stage;

// world shader types
#define VULKAN_OBJECT_MAX_COUNT 1024

#define VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT 2 // descriptor per object

#define VULKAN_OBJECT_SHADER_SAMPLER_COUNT 2

typedef struct descriptor_id_track
{
  u32 id[3];
} descriptor_id_track;
#define VULKAN_MAX_GEOMETRY_COUNT 1024
typedef struct vulkan_descriptor_state
{
  u32 generation[3];
  u32 id[3];
  descriptor_id_track ids[VULKAN_OBJECT_SHADER_SAMPLER_COUNT];
} vulkan_descriptor_state;

typedef struct vulkan_objcet_shader_object_state
{
  // per frame
  VkDescriptorSet descriptor_sets[3];
  bool fully_updated;
  // per descriptor
  vulkan_descriptor_state descriptor_state[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT];
} vulkan_objcet_shader_object_state;

typedef struct vulkan_shader_object
{
  // vertext and fragment shader object
  vulkan_shader_stage shader_stages[2];
  // global descriptors
  VkDescriptorSetLayout global_descriptor_set_layout;
  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSet global_descriptor_set[3]; // we are using three for triple buffering

  global_uniform_object global_ubo;
  vulkan_buffer global_uniform_buffer;
  vulkan_pipeline pipeline;

  // local descriptors dealing per object
  VkDescriptorSetLayout object_descriptor_set_layout;
  VkDescriptorPool object_descriptor_pool;
  vulkan_buffer object_uniform_buffer;
  u32 object_uniform_buffer_index;
  vulkan_objcet_shader_object_state object_states[VULKAN_OBJECT_MAX_COUNT];
  texture_use sampler_uses[3];
} vulkan_shader_object;

// UI shader types
#define VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT 2

#define VULKAN_OBJECT_SHADER_UI_SAMPLER_COUNT 1

#define VULKAN_OBJECT_UI_MAX_COUNT 1024

typedef struct vulkan_objcet_shader_UI_object_state
{
  // per frame
  VkDescriptorSet descriptor_sets[3];
  // per descriptor
  vulkan_descriptor_state descriptor_state[VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT];
} vulkan_objcet_shader_UI_object_state;

typedef struct vulkan_shader_UI_object
{
  // vertext and fragment shader object
  vulkan_shader_stage shader_stages[2];
  // global descriptors
  VkDescriptorSetLayout global_descriptor_set_layout;
  VkDescriptorPool global_descriptor_pool;
  VkDescriptorSet global_descriptor_set[3]; // we are using three for triple buffering

  global_UI_uniform_object global_ubo;
  vulkan_buffer global_uniform_buffer;
  vulkan_pipeline pipeline;

  // local descriptors dealing per object
  VkDescriptorSetLayout object_descriptor_set_layout;
  VkDescriptorPool object_descriptor_pool;
  vulkan_buffer object_uniform_buffer;
  u32 object_uniform_buffer_index;
  vulkan_objcet_shader_UI_object_state object_states[VULKAN_OBJECT_UI_MAX_COUNT];
  texture_use sampler_uses[3];
} vulkan_shader_UI_object;

typedef struct vulkan_geometry_data
{
  u32 id;
  u32 generations;
  u32 vertex_count;
  u32 vertex_element_size;
  u64 vertex_buffer_offset;
  u32 index_count;
  u32 index_element_size;
  u64 index_buffer_offset;
} vulkan_geometry_data;

typedef struct vulkan_texture_data
{
  vulkan_image image;
  VkSampler sampler;
} vulkan_texture_data;

// New shader impelmentation
#define VULKAN_SHADER_MAX_PUSH_CONST_RANGES 2
#define VULKAN_SHADER_MAX_STAGE 2
#define VULKAN_SHADER_MAX_GLOBAL_TEXTURE 2
#define VULKAN_SHADER_MAX_ATTRIBUTES 2
#define VULKAN_SHADER_MAX_UNIFORM 1024
#define VULKAN_SHADER_MAX_INSTANCE_TEXTURE 1024
#define VULKAN_SHADER_MAX_BINDINGS 2
#define VULKAN_MAX_SHADER 1024

enum vulkan_shader_state
{
  VULKAN_SHADER_STATE_NOT_CREATED,

  VULKAN_SHADER_STATE_UNINITIALIZED,

  VULKAN_SHADER_STATE_INITIALIZED,
};

typedef struct vulkan_shader_stage_config
{
  char stage_str[7];
  VkShaderStageFlagBits stage_flag;
} vulkan_shader_stage_config;

typedef struct vulkan_shader_stage_new
{
  VkPipelineShaderStageCreateInfo shader_satge_create_info;
  VkShaderModule handle;
  VkShaderModuleCreateInfo shader_module_create_info;
} vulkan_shader_stage_new;

typedef struct vulkan_descriptor_set_config
{
  VkDescriptorSetLayoutBinding binding[2];
  u64 binding_count;
} vulkan_descriptor_set_config;

typedef struct vulkan_descriptor_state_new
{
  u32 generations[3];
  u32 id[3];
} vulkan_descriptor_state_new;

typedef struct vulkan_descriptor_set_state
{
  VkDescriptorSet descriptor_set[3];
  vulkan_descriptor_state_new descriptor_state[2];
} vulkan_descriptor_set_state;

typedef struct vulkan_uniform_lookup_entry
{
  shader_scope scope;
  u64 location;
  u64 size;
  u64 offset;
  u32 index;
  u32 set_index;
} vulkan_uniform_lookup_entry;

typedef struct range
{
  //    mat4 push_range;
  u64 offset;
  u64 size;
} range;

typedef struct shader_config
{
  u64 attribute_stride;
  u64 push_constant_range_count;
  range push_constant_range[VULKAN_SHADER_MAX_PUSH_CONST_RANGES];
  u64 max_desrciptor_set_count;
  vulkan_shader_stage_config stages[VULKAN_SHADER_MAX_STAGE];
  u64 descriptor_set_count;
  vulkan_descriptor_set_config descriptor_sets[2];
  VkDescriptorPoolSize pool_size[2];
  u64 stage_count;
  VkVertexInputAttributeDescription attributes[VULKAN_SHADER_MAX_ATTRIBUTES];
  u64 attribute_count;
} shader_config;

typedef struct vulkan_shader_instance_state
{
  u64 id;
  u64 offset;
  Texture *instance_texture[2];
  vulkan_descriptor_set_state descriptor_set_state;
} vulkan_shader_instance_state;

typedef struct vulkan_shader
{
  u32 internal_id;
  struct vulkan_context *context;
  vulkan_shader_state state;
  const char *name;
  bool use_instance;
  bool use_push_constants;
  range push_constant_range[VULKAN_SHADER_MAX_PUSH_CONST_RANGES];
  VkDescriptorSetLayout descriptor_set_layout[3];
  VkDescriptorPool descriptor_pool;
  VkDescriptorSet global_descriptor_set[3];
  vulkan_renderpass *renderpass;
  shader_config config;
  vulkan_shader_stage_new stages[VULKAN_SHADER_MAX_STAGE];
  vulkan_shader_instance_state instance_state[1024];
  u64 bound_instance_id;
  Texture *global_texture[VULKAN_SHADER_MAX_GLOBAL_TEXTURE];
  u32 global_texture_count;
  u32 instance_texture_count;
  vulkan_buffer uniform_buffer;
  void *mapped_uniform_buffer_block;
  vulkan_uniform_lookup_entry uniforms[VULKAN_SHADER_MAX_UNIFORM];
  u64 uniform_count;
  hashtable uniform_look_up;
  void *hash_table_block;
  u64 global_ubo_size;
  u64 global_ubo_stride;
  u64 global_ubo_offset;
  u64 bound_ubo_offset;
  u64 ubo_size;
  u64 ubo_stride;
  u64 required_ubo_alignment;
  u64 push_constant_stride;
  u64 push_constant_size;
  vulkan_pipeline pipeline;
} vulkan_shader;

// New shader impelmentation

typedef struct vulkan_context
{
  float frame_delta_time;

  VkInstance instance;

  VkAllocationCallbacks *allocator;

  VkSurfaceKHR surface;

#ifndef NDEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif

  vulkan_device device;

  u64 framebuffer_width;

  u64 framebuffer_height;

  u64 framebuffer_size_generation;

  u64 framebuffer_size_last_generation;

  vulkan_swapchain swapchain;

  vulkan_frame_buffers UI_frame_buffers[3];

  u32 image_index;

  u64 current_frame;

  bool recreate;
  vulkan_command_buffer *graphics_command_buffers;

  vulkan_renderpass main_renderpass;
  vulkan_renderpass UI_renderpass;

  VkSemaphore *image_available_semaphore;

  VkSemaphore *queue_complete_semaphore;

  u64 in_flight_fence_count;

  vulkan_fence *in_flight_fences;

  vulkan_fence **images_in_flight;

  vulkan_buffer vertext_buffer;

  vulkan_buffer index_buffer;

  int (*find_memory_index)(u32 type_filter, u32 property_flag);

  vulkan_shader_object built_in_object_shader;
  vulkan_shader_object light_shader;
  vulkan_renderpass light_renderpass;
  vulkan_frame_buffers light_framebuffers[3];
  vulkan_shader_UI_object built_in_UI_object_shader;

  vulkan_geometry_data geometries[VULKAN_MAX_GEOMETRY_COUNT];
 
  str_to_enum array[28];
  // temp
  vulkan_shader test_shader;
  vulkan_shader shaders[VULKAN_MAX_SHADER];
  u32 test_shader_projection_location;
  u32 test_shader_view_location;
  u32 test_shader_diffuse_color_location;
  u32 test_shader_diffuse_texture_location;
  u32 test_shader_model_location;
  // temp

} vulkan_context;