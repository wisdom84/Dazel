#include "vulkan_backend.h"
#include "Math/dlm.h"
#include "resources/resources.inl"
#include "Renderer/renderer_types.inl"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_image.h"
#include "vulkan_renderpass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"
#include "shaders/vulkan_object_shaders.h"
#include "shaders/vulkan_object_ui_shaders.h"
#include "vulkan_buffer.h"
#include "core/logger.h"
#include "vulkan_types.inl"
#include "core/Dstrings.h"
#include "core/Dmemory.h"
#include "containers/Darray.h"
#include "vulkan_platform.h"
#include "core/application.h"
#include "systems/material_system.h"
#include "systems/shader_system.h"
#include "Renderer/vulkan/shaders/vulkan_shader.h"
using namespace dlm;
// Renderer render_object;
struct vulkan_context context;
static u64 cached_framebuffer_width = 0;
static u64 cached_framebuffer_height = 0;
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *pcallbackData,
    void *puserdata);
int find_memory_index(u32 type_filter, u32 property_flag);

void create_command_buffers(renderer_backend *backend);
void regenerate_framebuffers(renderer_backend *backend, vulkan_frame_buffers *frame_buffers, vulkan_renderpass *renderpass, u64 attachment_count, VkImageView *attachments);
bool recreate_swapchain(renderer_backend *backend);
bool create_buffers(vulkan_context *context);
void set_shader_attribute_types();
void add_uniform(vulkan_shader *shader, shader_scope scope, const char *type, const char *name, u32 *out_location);
shader_attribute_type change_str_to_enum(const char *str);
void add_shader_property(const char *property_name, material_shader_config config, vulkan_shader *shader, material_shader *material_shader);
void set_uniform(vulkan_shader *shader, const char *type, u32 location, void *value);
VkShaderStageFlagBits set_shader_stages(char stages[][MAX_STAGE_NAME_LENGTH], u32 stage_count);
#define ADD_UNIFORM(shader, scope, type, name, location) add_uniform(shader, scope, type, name, location);
#define SET_UNIFORM(shader, type, location, value) set_uniform(shader, type, location, value);
// bool create_UI_buffers(vulkan_context *context);

bool upload_data_range(vulkan_context *context, VkCommandPool pool, VkFence fence, VkQueue queue, vulkan_buffer *buffer, u64 *offset, u64 size, const void *data)
{
     if (!vulkan_buffer_allocate(buffer, size, offset))
     {
          DERROR("upload data range failed to allocate from a given buffer");
          return false;
     }
     // create a staging buffer
     VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
     VkMemoryPropertyFlags mem_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
     vulkan_buffer staging_buffer;
     vulkan_buffer_create(context, size, mem_flags, true, buffer_usage_flags, &staging_buffer);
     // load data to our staging buffer
     vulkan_buffer_load_data(context, &staging_buffer, 0, size, 0, data);
     // copy data from staging buffer to vertex buffer
     vulkan_buffer_copy(context, pool, fence, queue, &staging_buffer.handle, 0, &buffer->handle, *offset, size);
     // destroy staging buffer
     vulkan_buffer_destroy(context, &staging_buffer);
     return true;
}

void free_data_range(vulkan_buffer *buffer, u64 offset, u64 size)
{
     // TODO:
     if (buffer)
     {
          vulkan_buffer_free(buffer, size, offset);
     }
}

bool vulkan_backend_initialize(struct renderer_backend *backend, const char *application_name, struct platform_state *plat_state, int width, int height)
{
     context.allocator = 0;
     context.find_memory_index = find_memory_index;

     application_get_framebuffer(&cached_framebuffer_width, &cached_framebuffer_height);
     context.framebuffer_width = (cached_framebuffer_width != 0) ? cached_framebuffer_width : 800;
     context.framebuffer_height = (cached_framebuffer_height != 0) ? cached_framebuffer_height : 600;
     cached_framebuffer_width = 0;
     cached_framebuffer_height = 0;

     // set up vulkan instance
     VkApplicationInfo app_info;
     app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
     app_info.apiVersion = VK_API_VERSION_1_3;
     app_info.pApplicationName = application_name;
     app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
     app_info.pEngineName = "Dazel Engine";
     app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
     app_info.pNext = nullptr;
     VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
     create_info.pApplicationInfo = &app_info;

     // obtain list of required extensions
     const char **required_extensions = darray_create(const char *);
     darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME, const char *);
     platform_get_required_extensions(&required_extensions);

#ifndef NDEBUG
     darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME, const char *);
     DDEBUG("required extensions");
     u32 length = darray_get_length(required_extensions);
     for (u32 i = 0; i < length; i++)
     {
          DDEBUG(required_extensions[i]);
     }
#endif

     create_info.enabledExtensionCount = darray_get_length(required_extensions);
     create_info.ppEnabledExtensionNames = required_extensions;
     // validation layers
     const char **required_validation_layer_names = 0;
     u32 required_validation_layer_count = 0;
#ifndef NDEBUG
     DINFO("validation layers enabled enumerating");
     required_validation_layer_names = darray_create(const char *);
     darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation", const char *);
     required_validation_layer_count = darray_get_length(required_validation_layer_names);

     // obtain a list of a available validation layers
     uint32_t available_layer_count = 0;
     vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);
     VkLayerProperties *available_validation_layer_names = darray_reservered(available_layer_count, VkLayerProperties);
     vkEnumerateInstanceLayerProperties(&available_layer_count, available_validation_layer_names);
     for (u32 i = 0; i < required_validation_layer_count; i++)
     {
          DINFO("searching for layer: %s...", required_validation_layer_names[i]);
          bool found = false;
          for (u32 j = 0; j < available_layer_count; j++)
          {
               if (string_equal(required_validation_layer_names[i], available_validation_layer_names[j].layerName))
               {
                    found = true;
                    DINFO("Found");
                    break;
               }
          }
          if (!found)
          {
               DFATAL("required validation layer is missing: %s", required_validation_layer_names[i]);
               return false;
          }
          DINFO("All Required validation layers are present");
     }
#endif
     create_info.enabledLayerCount = required_validation_layer_count;
     create_info.ppEnabledLayerNames = required_validation_layer_names;
     create_info.pNext = nullptr;
     create_info.flags = 0;

     VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));

     DINFO("instance created");
// Debugger
#ifndef NDEBUG
     DDEBUG("creating the vulkan debugger");
     u32 long_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
     VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
     debug_create_info.messageSeverity = long_severity;
     debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
     debug_create_info.pfnUserCallback = vk_debug_callback;
     debug_create_info.pUserData = nullptr;
     debug_create_info.flags = 0;
     PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
         context.instance, "vkCreateDebugUtilsMessengerEXT");
     DASSERT_MSG(func, "failed to create  debug messenger!");
     VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
     DDEBUG("vulkan debugger created");
#endif
     // create surface
     platform_create_vulkan_surface(plat_state, &context);
     // create physical device
     if (!vulkan_device_create(&context))
     {
          DERROR("failed to get physical device");
          return false;
     }
     // create swapchain
     vulkan_swapchain_create(&context, &context.framebuffer_width, &context.framebuffer_height, &context.swapchain);

     // create renderpass

     vulkan_renderpass_create(
         &context,
         &context.main_renderpass,
         0.0f, 0.0f, context.swapchain.extent.width, context.swapchain.extent.height,
         0.0f, 0.0f, 0.2f, 1.0f,
         1.0f,
         0,
         RENDERPASS_CLEAR_COLOR_BUFFER_FLAG | RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG | RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG,
         false,
         false);

     vulkan_renderpass_create(
         &context,
         &context.light_renderpass,
         0.0f, 0.0f, context.swapchain.extent.width, context.swapchain.extent.height,
         0.0f, 0.0f, 0.2f, 1.0f,
         1.0f,
         0,
         RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG,
         true,
         false);

     vulkan_renderpass_create(
         &context,
         &context.UI_renderpass,
         0.0f, 0.0f, context.swapchain.extent.width, context.swapchain.extent.height,
         0.0f, 0.0f, 0.2f, 1.0f,
         1.0f,
         0,
         RENDERPASS_CLEAR_NONE_FLAG,
         true, false);

     if (context.main_renderpass.handle)
     {
          DINFO("Render pass is valid");
     }
     // swapchain frame buffers
     context.swapchain.frame_buffers = darray_reservered(context.swapchain.image_count, vulkan_frame_buffers);

     for (u32 i = 0; i < context.swapchain.image_count; i++)
     {
          VkImageView Attachments[2] = {context.swapchain.image_views[i], context.swapchain.depth_attachment.view};
          VkImageView UI_attachments[1] = {context.swapchain.image_views[i]};
          regenerate_framebuffers(backend, &context.swapchain.frame_buffers[i], &context.main_renderpass, 2, Attachments);
          regenerate_framebuffers(backend, &context.light_framebuffers[i], &context.light_renderpass, 2, Attachments);
          regenerate_framebuffers(backend, &context.UI_frame_buffers[i], &context.UI_renderpass, 1, UI_attachments);
     }

     // create command buffers
     create_command_buffers(backend);

     // create synchronization objects
     context.image_available_semaphore = darray_reservered(context.swapchain.max_frames_in_flight, VkSemaphore);
     context.queue_complete_semaphore = darray_reservered(context.swapchain.max_frames_in_flight, VkSemaphore);
     context.in_flight_fences = darray_reservered(context.swapchain.max_frames_in_flight, vulkan_fence);
     for (u64 i = 0; i < context.swapchain.max_frames_in_flight; i++)
     {
          VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
          vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.image_available_semaphore[i]);
          vkCreateSemaphore(context.device.logical_device, &semaphore_create_info, context.allocator, &context.queue_complete_semaphore[i]);
          vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
     }
     context.images_in_flight = darray_reservered(context.swapchain.image_count, vulkan_fence *);
     for (u64 i = 0; i < context.swapchain.image_count; i++)
     {
          context.images_in_flight[i] = 0;
     }
     DINFO("synchronization objects created successfully");
     for (u32 i = 0; i < VULKAN_MAX_SHADER; i++)
     {
          context.shaders[i].internal_id = INVALID_ID;
     }

     if (create_buffers(&context))
     {
          DINFO("buffers created successfully");
     };
     // test code
     for (u32 i = 0; i < VULKAN_MAX_GEOMETRY_COUNT; i++)
     {
          context.geometries[i].id = INVALID_ID;
          context.geometries[i].generations = INVALID_ID;
     }
     DINFO("vulkan intialized successfully");
     set_shader_attribute_types();
     context.geometry_vertex_offset = 0;
     context.geometry_index_offset = 0;
     return true;
};

void set_shader_attribute_types()
{
     const char *array[]{
         "FLOAT32",
         "FLOAT32_2",
         "FLOAT32_3",
         "FLOAT32_4",
         "INT8",
         "INT8_2",
         "INT8_3",
         "INT8_4",
         "UINT8",
         "UINT8_2",
         "UINT8_3",
         "UINT8_4",
         "INT16",
         "INT16_2",
         "INT16_3",
         "INT16_4",
         "UINT16",
         "UINT16_2",
         "UINT16_3",
         "UINT16_4",
         "INT32",
         "INT32_2",
         "INT32_3",
         "INT32_4",
         "UINT32",
         "UINT32_2",
         "UINT32_3",
         "UINT32_4"};
     for (u32 i = 0; i < 28; i++)
     {
          string_copy(context.array[i].str_variant, array[i]);
          context.array[i].value = (shader_attribute_type)(SHADER_ATTRIBUTE_TYPE_FLOAT32 + i);
     }
}

void vulkan_backend_shutdown(struct renderer_backend *backend)
{
     //    render_object.cleanup();
     vkDeviceWaitIdle(context.device.logical_device);
     // Destroy Buffers
     DDEBUG("Destroying vulkan buffers");
     vulkan_buffer_destroy(&context, &context.vertext_buffer);
     vulkan_buffer_destroy(&context, &context.index_buffer);
     DDEBUG("Destroying synchronization objects")
     for (u64 i = 0; i < context.swapchain.max_frames_in_flight; i++)
     {
          if (context.image_available_semaphore[i])
          {
               vkDestroySemaphore(
                   context.device.logical_device,
                   context.image_available_semaphore[i],
                   context.allocator);
               context.image_available_semaphore[i] = 0;
          }
          if (context.queue_complete_semaphore[i])
          {
               vkDestroySemaphore(
                   context.device.logical_device,
                   context.queue_complete_semaphore[i],
                   context.allocator);
               context.queue_complete_semaphore[i] = 0;
          }
          vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
     }
     darray_destroy_array(context.image_available_semaphore);
     context.image_available_semaphore = 0;

     darray_destroy_array(context.queue_complete_semaphore);
     context.queue_complete_semaphore = 0;

     darray_destroy_array(context.in_flight_fences);
     context.in_flight_fences = 0;

     darray_destroy_array(context.images_in_flight);
     context.images_in_flight = 0;

     // Destroying command pool
     DDEBUG("freeing the command buffers");
     for (u64 i = 0; i < context.swapchain.image_count; i++)
     {
          if (context.graphics_command_buffers[i].handle)
          {
               vulkan_command_buffer_free(&context, context.device.graphics_command_pool, (context.graphics_command_buffers + i));
               context.graphics_command_buffers[i].handle = 0;
          }
     }
     darray_destroy_array(context.graphics_command_buffers);
     // Destroy framebuffers
     DDEBUG("Destroying frame buffers");
     for (u64 i = 0; i < context.swapchain.image_count; i++)
     {
          vulkan_framebuffer_destroy(&context, &context.swapchain.frame_buffers[i]);
          vulkan_framebuffer_destroy(&context, &context.UI_frame_buffers[i]);
          vulkan_framebuffer_destroy(&context, &context.light_framebuffers[i]);
     }
     darray_destroy_array(context.swapchain.frame_buffers);
     // Renderpass destroy
     DDEBUG("Destroying renderpass");
     vulkan_renderpass_destroy(&context, &context.main_renderpass);
     vulkan_renderpass_destroy(&context, &context.UI_renderpass);
     vulkan_renderpass_destroy(&context, &context.light_renderpass);
     // swapchain destroy
     DDEBUG("Destroying swapchain resources");
     vulkan_swapchain_destroy(&context, &context.swapchain);
     DDEBUG("Destroying vulkan device");
     vulkan_device_destroy(&context);
     DDEBUG("destroying vulkan surface");
     vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
#ifndef NDEBUG
     DDEBUG("Destroying the debugger");
     if (context.debug_messenger)
     {
          PFN_vkDestroyDebugUtilsMessengerEXT func =
              (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
          func(context.instance, context.debug_messenger, context.allocator);
     }
#endif
     DDEBUG("Destroying vulkan instance");
     vkDestroyInstance(context.instance, context.allocator);
};

void vulkan_backend_resized(struct renderer_backend *backend, u16 width, u16 height)
{
     cached_framebuffer_height = height;
     cached_framebuffer_width = width;
     context.framebuffer_size_generation++;
     DINFO("vulkan renderer backend->resized: width: %i, height: %i, frame_size_gen: %i", width, height, context.framebuffer_size_generation);
};

bool vulkan_backend_begin_frame(struct renderer_backend *backend, float delta_time)
{
     context.frame_delta_time = delta_time;
     vulkan_device *device = &context.device;
     // check if recreating swapchain
     if (context.recreate)
     {
          VkResult result = vkDeviceWaitIdle(device->logical_device);
          if (!vulkan_result_is_success(result))
          {
               DERROR("vulkan_backend_begin_frame vkDeviceWaitIdle failed: '%s',", vulkan_result_string(result, true));
               return false;
          }
     }
     // check if the framebuffer has been resized
     if (context.framebuffer_size_generation != context.framebuffer_size_last_generation)
     {
          VkResult result = vkDeviceWaitIdle(device->logical_device);
          if (!vulkan_result_is_success(result))
          {
               DERROR("vulkan_backend_begin_frame vkDeviceWaitIdle failed: '%s',", vulkan_result_string(result, true));
               return false;
          }

          // if the swapchain failed to be recreated
          if (!recreate_swapchain(backend))
          {
               return false;
          }
          DINFO("Resized successful booting...");
     }

     // wait for the execution of the current frame to complete
     if (!vulkan_fence_wait(&context, &context.in_flight_fences[context.current_frame], UINT64_MAX))
     {
          DWARNING("in-flight fence wait failure");
          return false;
     }
     // Aquire an image from the swapchain

     if (!vulkan_swapchain_aquire_next_image_index(
             &context,
             &context.swapchain,
             UINT64_MAX,
             context.image_available_semaphore[context.current_frame],
             nullptr,
             &context.image_index))
     {
          return false;
     }
     // begin recording command buffer
     vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];
     vulkan_command_buffer_reset(command_buffer);
     vulkan_command_buffer_begin(command_buffer, false, false, false);
     // dynamic states

     // view port
     VkViewport viewport{};
     viewport.x = 0.0; // the region in the frame buffer we want to start excuting our draw operation. in this case we are starting from the origin
     viewport.y = (float)context.swapchain.extent.height;
     viewport.width = (float)context.swapchain.extent.width;
     viewport.height = -(float)context.swapchain.extent.height;
     viewport.minDepth = 0.0f; // mindepth and maxdepth of the images are in the bounds (0-1) if we want to use depth we need a GPU feature
     viewport.maxDepth = 1.0f;
     // scissor
     VkRect2D scissor{};
     scissor.extent.width = context.swapchain.extent.width;
     scissor.extent.height = context.swapchain.extent.height;
     scissor.offset.x = 0;
     scissor.offset.y = 0;
     vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
     vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);
     context.main_renderpass.w = context.swapchain.extent.width;
     context.main_renderpass.h = context.swapchain.extent.height;
     context.UI_renderpass.w = context.swapchain.extent.width;
     context.UI_renderpass.h = context.swapchain.extent.height;
     context.light_renderpass.w = context.swapchain.extent.width;
     context.light_renderpass.h = context.swapchain.extent.height;
     return true;
};
void vulkan_renderer_update_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, int mode, float delta_time, global_uniform_object*object, u32 shader_id)
{
          material_shader shader;
          shader_system_get_shader_by_id(shader_id, &shader);
          vulkan_shader *internal_data = (vulkan_shader *)shader.internal_data;
          vulkan_shader_use(&context.shaders[internal_data->internal_id]);
          vulkan_shader_bind_globals(&context.shaders[internal_data->internal_id]);
          global_uniform_object*data=object;
          u32 offset = 0;
          for (u32 i = 0; i < shader.config.uniform_count; i++)
          {
                    u32 location;
                    u32 size;
                    if (string_equal(shader.config.uniform_scope[i], "GLOBAL"))
                    {
                              location = vulkan_shader_uniform_location(&context.shaders[internal_data->internal_id], shader.config.uniform_names[i]);
                              size =vulkan_shader_get_uniform_size(&context.shaders[internal_data->internal_id], location);
                              SET_UNIFORM(&context.shaders[internal_data->internal_id], shader.config.uniform_type[i], location, (char*)data+offset);
                              offset += size;
                    };
          }
         vulkan_shader_apply_globals(&context.shaders[internal_data->internal_id]);
}
void vulkan_renderer_update_light_global_state(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_color, int mode, float delta_time)
{
     // context.light_shader.global_ubo.projection = projection;
     // context.light_shader.global_ubo.view = view;
     // context.light_shader.global_ubo.view_position = view_position;
     // context.light_shader.global_ubo.ambient_color = ambient_color;
     // vulkan_shader_object_update_global_state(&context, &context.light_shader, delta_time);
}

void vulkan_renderer_update_UI_global_state(mat4 projection, mat4 view, int mode, float delta_time)
{
     context.built_in_UI_object_shader.global_ubo.projection = projection;
     context.built_in_UI_object_shader.global_ubo.view = view;
     vulkan_shader_object_update_global_UI_state(&context, &context.built_in_UI_object_shader, delta_time);
}

bool vulkan_backend_end_frame(struct renderer_backend *backend, float delta_time)
{
     vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];

     vulkan_command_buffer_end(command_buffer);
     // make sure the previous frame is not using this image  (i.e its fence is being waited on)
     if (context.images_in_flight[context.image_index] != VK_NULL_HANDLE)
     {
          vulkan_fence_wait(
              &context,
              context.images_in_flight[context.image_index],
              UINT64_MAX);
     }
     context.images_in_flight[context.image_index] = &context.in_flight_fences[context.current_frame];
     vulkan_fence_reset(&context, &context.in_flight_fences[context.current_frame]);

     VkPipelineStageFlags pipeline_stage[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
     VkSubmitInfo Queue_submit_info;
     Queue_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
     Queue_submit_info.commandBufferCount = 1;
     Queue_submit_info.pCommandBuffers = &command_buffer->handle;

     Queue_submit_info.pNext = nullptr;
     Queue_submit_info.waitSemaphoreCount = 1;
     Queue_submit_info.pWaitSemaphores = &context.image_available_semaphore[context.current_frame];

     Queue_submit_info.pWaitDstStageMask = pipeline_stage;

     Queue_submit_info.signalSemaphoreCount = 1;
     Queue_submit_info.pSignalSemaphores = &context.queue_complete_semaphore[context.current_frame];

     VkResult result = vkQueueSubmit(
         context.device.graphics,
         1,
         &Queue_submit_info,
         context.in_flight_fences[context.current_frame].handle);
     if (result != VK_SUCCESS)
     {
          DERROR("vkQueue submit failed with the result : %s", vulkan_result_string(result, true));
          return false;
     }
     vulakn_command_buffer_update_submitted(command_buffer);

     vulkan_swapchain_present(
         &context,
         &context.swapchain,
         context.device.graphics,
         context.device.present,
         context.queue_complete_semaphore[context.current_frame],
         &context.image_index);

     return true;
};

bool vulkan_renderer_begin_renderpass(struct renderer_backend *backend, u8 renderpass_id)
{
     vulkan_renderpass *renderpass = 0;
     vulkan_frame_buffers framebuffer;
     vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];

     switch (renderpass_id)
     {
     case BUILTIN_RENDERPASS_WORLD:
          renderpass = &context.main_renderpass;
          framebuffer = context.swapchain.frame_buffers[context.image_index];
          break;
     case BUILTIN_RENDERPASS_UI:
          renderpass = &context.UI_renderpass;
          framebuffer = context.UI_frame_buffers[context.image_index];
          break;
     case BUILTIN_RENDERPASS_LIGHT:
          renderpass = &context.light_renderpass;
          framebuffer = context.light_framebuffers[context.image_index];
          break;
     default:
          DERROR("begin renderpass called with an invalid id");
          return false;
     }
     // begin our choosen renderpass
     vulkan_renderpass_begin(command_buffer, renderpass, framebuffer.handle);
     return true;
};

bool vulkan_renderer_end_renderpass(struct renderer_backend *backend, u8 renderpass_id)
{
     vulkan_renderpass *renderpass = 0;
     vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];
     switch (renderpass_id)
     {
     case BUILTIN_RENDERPASS_WORLD:
          renderpass = &context.main_renderpass;
          break;
     case BUILTIN_RENDERPASS_UI:
          renderpass = &context.UI_renderpass;
          break;
     case BUILTIN_RENDERPASS_LIGHT:
          renderpass = &context.light_renderpass;
          break;
     default:
          DERROR("end renderpass called with an invalid id");
          return false;
     }
     vulkan_renderpass_end(command_buffer, renderpass);
     return true;
};

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *pcallbackData,
    void *puserdata)
{
     if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
     {
          DERROR("validation layer: %s", pcallbackData->pMessage);
     }
     else if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
     {
          DINFO("validation layer: %s", pcallbackData->pMessage);
     }
     else if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
     {
          DWARNING("validation layer: %s", pcallbackData->pMessage);
     }
     else if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
     {
          DTRACE("validation layer: %s", pcallbackData->pMessage);
     }
     return VK_FALSE;
};

int find_memory_index(u32 type_filter, u32 property_flag)
{
     VkPhysicalDeviceMemoryProperties memory_properties;
     vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);
     for (u32 i = 0; i < memory_properties.memoryTypeCount; i++)
     {
          if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flag) == property_flag)
          {
               return i;
          }
     }
     DWARNING("Unable to find suitable memory type!");
     return -1;
}

void create_command_buffers(renderer_backend *backend)
{
     if (!context.graphics_command_buffers)
     {
          context.graphics_command_buffers = darray_reservered(context.swapchain.image_count, vulkan_command_buffer);
          for (u32 i = 0; i < context.swapchain.image_count; i++)
          {
               Dzero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
          }
     }
     for (u32 i = 0; i < context.swapchain.image_count; i++)
     {
          if (context.graphics_command_buffers[i].handle)
          {
               vulkan_command_buffer_free(
                   &context,
                   context.device.graphics_command_pool,
                   &context.graphics_command_buffers[i]);
          }
          Dzero_memory(&context.graphics_command_buffers[i], sizeof(vulkan_command_buffer));
          vulkan_command_buffer_allocate(
              &context,
              context.device.graphics_command_pool,
              true,
              &context.graphics_command_buffers[i]);
     }
     DINFO("command buffers created successfully");
}

void regenerate_framebuffers(renderer_backend *backend,
                             vulkan_frame_buffers *frame_buffers,
                             vulkan_renderpass *renderpass, u64 attachment_count,
                             VkImageView *attachments)
{
     vulkan_framebuffer_create(
         &context,
         context.swapchain.extent.width,
         context.swapchain.extent.height,
         attachment_count,
         attachments,
         renderpass,
         frame_buffers);
     DINFO("framebuffers created successfully");
};

bool recreate_swapchain(renderer_backend *backend)
{
     if (context.recreate)
     {
          DDEBUG("recreate called when already recreating boot..");
          return false;
     }
     // Detect if window is minimized
     if (context.framebuffer_width == 0 || context.framebuffer_height == 0)
     {
          DDEBUG("recreating_swapchain called when window is < 1 in a dimesion booting");
          return false;
     }
     context.recreate = true;
     vkDeviceWaitIdle(context.device.logical_device);
     for (u64 i = 0; i < context.swapchain.image_count; i++)
     {
          context.images_in_flight[i] = 0;
     }
     // Requery support
     vulkan_device_query_swapchain_support(
         context.device.physical_device,
         context.surface,
         &context.device.swapchain_support);
     vulkan_device_detect_depth_format(&context.device);

     vulkan_swapchain_recreate(
         &context,
         &cached_framebuffer_width,
         &cached_framebuffer_height,
         &context.swapchain);
     // sync framebuffer size with the cached size
     context.framebuffer_width = cached_framebuffer_width;
     context.framebuffer_height = cached_framebuffer_height;
     context.main_renderpass.w = context.swapchain.extent.width;
     context.main_renderpass.h = context.swapchain.extent.height;
     context.UI_renderpass.w = context.swapchain.extent.width;
     context.UI_renderpass.h = context.swapchain.extent.height;
     context.light_renderpass.w = context.swapchain.extent.width;
     context.light_renderpass.h = context.swapchain.extent.height;
     cached_framebuffer_width = 0;
     cached_framebuffer_height = 0;

     // update framebuffer size generation
     context.framebuffer_size_last_generation = context.framebuffer_size_generation;
     // clean up swapchain
     for (u64 i = 0; i < context.swapchain.image_count; i++)
     {
          vulkan_command_buffer_free(&context, context.device.graphics_command_pool, &context.graphics_command_buffers[i]);
     }

     // Framebuffers
     for (u64 i = 0; i < context.swapchain.image_count; i++)
     {
          vulkan_framebuffer_destroy(&context, &context.swapchain.frame_buffers[i]);
          vulkan_framebuffer_destroy(&context, &context.UI_frame_buffers[i]);
          vulkan_framebuffer_destroy(&context,&context.light_framebuffers[i]);
     }
     context.main_renderpass.x = 0.0f;
     context.main_renderpass.y = 0.0f;
     context.main_renderpass.w = context.swapchain.extent.width;
     context.main_renderpass.h = context.swapchain.extent.height;

     context.UI_renderpass.x = 0.0f;
     context.UI_renderpass.y = 0.0f;
     context.UI_renderpass.w = context.swapchain.extent.width;
     context.UI_renderpass.h = context.swapchain.extent.height;

     context.light_renderpass.x = 0.0f;
     context.light_renderpass.y = 0.0f;
     context.light_renderpass.w = context.swapchain.extent.width;
     context.light_renderpass.h = context.swapchain.extent.height;

     for (u32 i = 0; i < context.swapchain.image_count; i++)
     {
          VkImageView Attachments[2] = {context.swapchain.image_views[i], context.swapchain.depth_attachment.view};
          VkImageView UI_attachments[1] = {context.swapchain.image_views[i]};
          regenerate_framebuffers(backend, &context.swapchain.frame_buffers[i], &context.main_renderpass, 2, Attachments);
          regenerate_framebuffers(backend, &context.light_framebuffers[i], &context.light_renderpass, 2, Attachments);
          regenerate_framebuffers(backend, &context.UI_frame_buffers[i], &context.UI_renderpass, 1, UI_attachments);
     }

     create_command_buffers(backend);
     context.recreate = false;
     return true;
}

bool create_buffers(vulkan_context *context)
{
     VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
     u64 vertex_buffer_size = sizeof(Vertex_3d) * 1024 * 1024;
     if (!vulkan_buffer_create(
             context,
             vertex_buffer_size,
             memory_property_flags,
             true,
             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
             &context->vertext_buffer))
     {
          DERROR("failed to create vertex buffer");
          return false;
     }
     context->geometry_vertex_offset = 0;
     u64 index_buffer_size = sizeof(Vertex_3d) * 1024 * 1024;
     if (!vulkan_buffer_create(
             context,
             index_buffer_size,
             memory_property_flags,
             true,
             VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
             &context->index_buffer))
     {
          DERROR("failed to create index buffer");
          return false;
     }
     context->geometry_index_offset = 0;
     return true;
}

void vulkan_create_texture(const char *name, int width, int height, int channel_count, const u8 *pixels, bool has_transparency, struct Texture *out_texture)
{
     out_texture->channel_count = channel_count;
     out_texture->width = width;
     out_texture->height = height;
     out_texture->has_transparency = has_transparency;
     out_texture->generation = INVALID_ID;
     string_n_copy(out_texture->name, name, TEXTURE_NAME_MAX_LENGTH);

     out_texture->internal_data = (vulkan_texture_data *)Dallocate_memory(sizeof(vulkan_texture_data), MEMORY_TAG_TEXTURE);
     vulkan_texture_data *texture_data = (vulkan_texture_data *)out_texture->internal_data;
     // create a buffer to hold the image infomation
     vulkan_buffer staging_buffer;
     VkDeviceSize image_size = width * height * channel_count;
     vulkan_buffer_create(&context, image_size,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          true, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &staging_buffer);
     // load data into the buffer
     vulkan_buffer_load_data(&context, &staging_buffer, 0, image_size, 0, pixels);

     // create an image
     VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
     VkImageUsageFlags image_usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
     vulkan_image_create(&context, width, height, format, VK_IMAGE_TILING_OPTIMAL, image_usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         VK_IMAGE_ASPECT_COLOR_BIT, &texture_data->image);
     // transition image layout  to transfer compatible
     vulkan_command_buffer command_buffer;

     vulkan_command_buffer_allocate_and_begin_single_use(&context, context.device.graphics_command_pool, &command_buffer);
     // transition image layout  to transfer compatible
     vulkan_image_transition_layout(&context, &command_buffer, &texture_data->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
     // copy buffer to image
     vulkan_image_copy_from_buffer(&context, &texture_data->image, &staging_buffer, &command_buffer);
     // transition image layout to shader compatible
     vulkan_image_transition_layout(&context, &command_buffer, &texture_data->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
     vulkan_command_buffer_end_single_use(&context, context.device.graphics_command_pool, &command_buffer, context.device.transfer);

     // set up a sampler
     VkSamplerCreateInfo texture_sampler_create_info;
     texture_sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
     texture_sampler_create_info.pNext = nullptr;
     texture_sampler_create_info.flags = 0;
     texture_sampler_create_info.magFilter = VK_FILTER_LINEAR; // this  defines how the texel coordinate are interpolated
     // when a texture image is maginfied very close to the screen
     texture_sampler_create_info.minFilter = VK_FILTER_LINEAR; // this defines how the texel coordinate are
     // when a texture image is minified (very far away form the screen)
     texture_sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // where we use the texel coordinate
     // for the x axis to determine how we draw pixels on our geometry when we reach the end of our texture picture in the x axis
     texture_sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;

     texture_sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
     // the remaining ones on the V and W are just the same effect that happpens on the x but just repeating
     // on the Y and Z
     texture_sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
     texture_sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // boder after reaching the end of our texture image
     texture_sampler_create_info.compareEnable = VK_FALSE;
     texture_sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS; // connected to the one in the depth buffer
     texture_sampler_create_info.mipLodBias = 0.0f;
     texture_sampler_create_info.maxLod = 0.0f;
     texture_sampler_create_info.minLod = 0.0f;
     texture_sampler_create_info.maxAnisotropy = 16.0f; // anisotropy level maping
     texture_sampler_create_info.unnormalizedCoordinates = VK_FALSE;
     texture_sampler_create_info.anisotropyEnable = VK_TRUE;
     VK_CHECK(vkCreateSampler(context.device.logical_device, &texture_sampler_create_info, context.allocator, &texture_data->sampler));
     //   out_texture->generation += 1;
     vulkan_buffer_destroy(&context, &staging_buffer);
};
void vulkan_destroy_texture(struct Texture *texture)
{
     vkDeviceWaitIdle(context.device.logical_device);
     if (texture)
     {

          if (texture->internal_data)
          {
               vulkan_texture_data *texture_data = (vulkan_texture_data *)texture->internal_data;
               vulkan_image_destroy(&context, &texture_data->image);
               vkDestroySampler(context.device.logical_device, texture_data->sampler, context.allocator);
               Dfree_memory(texture->internal_data, MEMORY_TAG_TEXTURE, sizeof(vulkan_texture_data));
          }
          texture->width = 0;
          texture->height = 0;
          texture->channel_count = 0;
          texture->has_transparency = false;
          texture->generation = 0;
     }
}
bool vulkan_create_geometry(geometry *geo_obj, u32 vertex_count, u32 vertex_size, const void *verticies, u32 index_size, u32 index_count, const void *indicies)
{
     if (!vertex_count || !verticies)
     {
          DERROR("vulkan_create_geometry requires vertex data none was provided vertex_count= %d, verticies = % p", vertex_count, verticies);
          return false;
     }
     // check if the geometry is for a reupload
     bool is_reupload = geo_obj->internal_id != INVALID_ID;
     vulkan_geometry_data old_range;

     vulkan_geometry_data *internal_data = 0;
     if (is_reupload)
     {
          internal_data = &context.geometries[geo_obj->internal_id];

          // take a copy of the old range
          old_range.index_buffer_offset = internal_data->index_buffer_offset;
          old_range.index_count = internal_data->index_count;
          old_range.index_element_size = internal_data->index_element_size;
          old_range.vertex_buffer_offset = internal_data->vertex_buffer_offset;
          old_range.vertex_count = internal_data->vertex_count;
          old_range.vertex_element_size = internal_data->vertex_element_size;
     }
     else
     {
          for (u32 i = 0; i < VULKAN_MAX_GEOMETRY_COUNT; i++)
          {
               if (context.geometries[i].id == INVALID_ID)
               {
                    geo_obj->internal_id = i;
                    context.geometries[i].id = i;
                    internal_data = &context.geometries[i];
                    break;
               }
          }
     }
     if (!internal_data)
     {
          DFATAL("vulkan_create_geometry failed to find a free slot to upload a new geometry  Adjust config to allow more upload of geometry to the vulkan backend api");
          return false;
     }
     VkCommandPool pool = context.device.graphics_command_pool;
     VkQueue queue = context.device.graphics;

     // Vertex data
     // internal_data->vertex_buffer_offset = context.geometry_vertex_offset;
     internal_data->vertex_count = vertex_count;
     internal_data->vertex_element_size = sizeof(Vertex_3d);
     u32 total_size = vertex_count * vertex_size;
     if (!upload_data_range(&context, pool, 0, queue, &context.vertext_buffer, &internal_data->vertex_buffer_offset, total_size, verticies))
     {
          DERROR("vulkan_renderer_create geometry failed to upload to the vertex buffer");
          return false;
     };
     //  context.geometry_vertex_offset += total_size;

     // index data if it exists
     if (index_count && indicies)
     {
          // internal_data->index_buffer_offset = context.geometry_index_offset;
          internal_data->index_count = index_count;
          internal_data->index_element_size = sizeof(u32);
          u32 total_size = index_count * index_size;
          if (!upload_data_range(&context, pool, 0, queue, &context.index_buffer, &internal_data->index_buffer_offset, total_size, indicies))
          {
               DERROR("vulkan_renderer_create geometry failed to upload to the index buffer");
               return false;
          };
          // context.geometry_index_offset += total_size;
     }
     if (internal_data->generations == INVALID_ID)
     {
          internal_data->generations = 0;
     }
     else
     {
          internal_data->generations++;
     }
     if (is_reupload)
     {
          free_data_range(&context.vertext_buffer, old_range.vertex_buffer_offset, old_range.vertex_count * old_range.vertex_element_size);
          // TODO: add  a free list to free vertex  data out of memory

          if (old_range.index_element_size > 0)
          {
               free_data_range(&context.index_buffer, old_range.index_buffer_offset, old_range.index_element_size * old_range.index_count);
               // TODO: free the index data out of memory if we had for that geometry
          }
     }
     return true;
};
void vulkan_destroy_geometry(geometry *geometry)
{
     if (geometry && geometry->internal_id != INVALID_ID)
     {
          vkDeviceWaitIdle(context.device.logical_device);
          vulkan_geometry_data *internal_data = &context.geometries[geometry->internal_id];

          // free  vertex_data
          // TODO: free vertex data
          if (internal_data->index_element_size > 0)
          {
              // TODO: free index data
          }
          // clean up memory
          Dzero_memory(internal_data, sizeof(vulkan_geometry_data));
          internal_data->id = INVALID_ID;
          internal_data->generations = INVALID_ID;
     }
};
void vulkan_renderer_draw_geometry(geometry_render_data data, u32 value)
{   if (data.geo_obj && data.geo_obj->internal_id == INVALID_ID)
     {
          return;
     }
     vulkan_geometry_data *buffer_data = &context.geometries[data.geo_obj->internal_id];
     vulkan_command_buffer *command_buffer = &context.graphics_command_buffers[context.image_index];
     //temp_code
       materials*default_m = material_system_get_default_material();
       Texture*t = default_m->diffuse_map.texture;
      //temp_code 
     // test code
     // binds the pipeline to the graphics bind point
     uniform_object ubo;
     materials *material;
     // bool apply_material;
     if (data.geo_obj->material)
     {
          material = data.geo_obj->material;
          // apply_material = data.geo_obj->apply_material;
     }
     else
     {
          material = material_system_get_default_material();
          // apply_material = data.geo_obj->apply_material;
     }
     ubo.model = data.model;
     ubo.diffuse_color = material->diffuse_color;
     ubo.shineness = material->shineness;
     ubo.sample_count_less = material->sample_count_less;
     // applying the material
     material_shader shader;
     shader_system_get_shader_by_id(material->shader_id, &shader);
     vulkan_shader *internal_data = (vulkan_shader *)shader.internal_data;
     vulkan_shader_bind_instance(&context.shaders[internal_data->internal_id], material->bound_instance_id);
      uniform_object*ubo_data = &ubo;
      u32 ubo_offset = 0;
     for (u32 i = 0; i < shader.config.uniform_count; i++)
     {
          u32 location;
          u32 size;
          if (string_equal(shader.config.uniform_scope[i], "LOCAL"))
          {
                    location = vulkan_shader_uniform_location(&context.shaders[internal_data->internal_id], shader.config.uniform_names[i]);
                    size =vulkan_shader_get_uniform_size(&context.shaders[internal_data->internal_id],location);
                    SET_UNIFORM(&context.shaders[internal_data->internal_id], shader.config.uniform_type[i], location, (char*)ubo_data+ubo_offset);
                    ubo_offset += size;       
          }
          else if (string_equal(shader.config.uniform_scope[i], "INSTANCE"))
          {
                  location = vulkan_shader_uniform_location(&context.shaders[internal_data->internal_id], shader.config.uniform_names[i]);
                  size =vulkan_shader_get_uniform_size(&context.shaders[internal_data->internal_id],location);
                  SET_UNIFORM(&context.shaders[internal_data->internal_id], shader.config.uniform_type[i], location, (char*)ubo_data+ubo_offset);
                  ubo_offset += size; 
          }
     }
     for (u32 i = 0; i < shader.config.sampler_count; i++)
     {
          u32 location;
          if (string_equal(shader.config.sampler_scope[i], "INSTANCE"))
          {
                    location = vulkan_shader_uniform_location(&context.shaders[internal_data->internal_id], shader.config.sampler_name[i]);
                    if(material->texture_maps[i].texture){
                          vulkan_shader_set_sampler(&context.shaders[internal_data->internal_id], location, material->texture_maps[i].texture);
                    }
                    else{
                         vulkan_shader_set_sampler(&context.shaders[internal_data->internal_id], location,t); 
                    }      
          }
     }
     if(material->renderer_frame_number != value){
        vulkan_shader_apply_instance(&context.shaders[internal_data->internal_id]); 
     }
     // bind the vertext buffer
     VkDeviceSize vertex_offset[1] = {buffer_data->vertex_buffer_offset};
     vkCmdBindVertexBuffers(command_buffer->handle, 0, 1, &context.vertext_buffer.handle, vertex_offset);
     if (buffer_data->index_count > 0)
     {
          // bind the index buffer
          VkDeviceSize index_offset = buffer_data->index_buffer_offset;
          vkCmdBindIndexBuffer(command_buffer->handle, context.index_buffer.handle, index_offset, VK_INDEX_TYPE_UINT32);
          // Issue a draw command
          vkCmdDrawIndexed(command_buffer->handle, buffer_data->index_count, 1, 0, 0, 0);
     }
     else
     {
          vkCmdDraw(command_buffer->handle, buffer_data->vertex_count, 1, 0, 0);
     }
}

VkShaderStageFlagBits set_shader_stages(char stages[4][MAX_STAGE_NAME_LENGTH], u32 stage_count)
{
     VkShaderStageFlagBits shader_stages = VK_SHADER_STAGE_VERTEX_BIT;
     for (u32 i = 0; i < stage_count; i++)
     {
          if (string_equal(stages[i], "fragment"))
          {
               shader_stages = (VkShaderStageFlagBits)(shader_stages | VK_SHADER_STAGE_FRAGMENT_BIT);
          }
          else if (string_equal(stages[i], "geometry"))
          {
               shader_stages = (VkShaderStageFlagBits)(shader_stages | VK_SHADER_STAGE_GEOMETRY_BIT);
          }
     }
     return shader_stages;
}

bool vulkan_create_material_shader(struct material_shader *shader)
{
     if (!shader)
     {
          DERROR("invalid shader object was passed to the backend ");
          return false;
     }
     VkShaderStageFlagBits stages;
     if (shader->config.stage_count > 0 && shader->config.stage_names[0])
     {
          stages = set_shader_stages(shader->config.stage_names, shader->config.stage_count);
     }
     else
     {
          DERROR("shader must have a list one shader stage for shader to be created");
          return false;
     }
     // initialize the shader
     for (u32 i = 0; i < VULKAN_MAX_SHADER; i++)
     {
          if (context.shaders[i].internal_id == INVALID_ID)
          {
               if (string_equal(shader->config.renderpass, "mainrenderpass"))
               {
                    if (!vulkan_shader_create(&context, shader->name, &context.main_renderpass, stages, shader->config.stage_file_names[0],shader->config.stage_file_names[1], 1024, shader->use_instance, shader->use_push_constants, &context.shaders[i]))
                    {
                         DERROR("vulkan failed to initialize internal_data for shader");
                         return false;
                    };
                    context.shaders[i].internal_id = i;
                    shader->internal_data = &context.shaders[i];
               }
               break;
          }
     }
     // add shader properties
     if (shader->config.attribute_count > 0)
     {
          add_shader_property("attributes", shader->config, (vulkan_shader *)shader->internal_data, shader);
     }
     if (shader->config.uniform_count > 0)
     {
          add_shader_property("uniforms", shader->config, (vulkan_shader *)shader->internal_data, shader);
     }
     if (shader->config.sampler_count > 0)
     {
          add_shader_property("samplers", shader->config, (vulkan_shader *)shader->internal_data, shader);
     }
     // intialize the shader
     if (!vulkan_shader_intialize((vulkan_shader *)shader->internal_data))
     {
          DERROR("could not initialize the vulkan material shader");
          return false;
     }
     return true;
};

void add_uniform(vulkan_shader *shader, shader_scope scope, const char *type, const char *name, u32 *out_location)
{
     if (string_equal(type, "mat4"))
          vulkan_shader_add_uniform_mat4(shader, name, scope, out_location);
     else if (string_equal(type, "vec4"))
          vulkan_shader_add_uniform_vec4(shader, name, scope, out_location);
     else if (string_equal(type, "i16"))
          vulkan_shader_add_uniform_i16(shader, name, scope, out_location);
     else if (string_equal(type, "i32"))
          vulkan_shader_add_uniform_i32(shader, name, scope, out_location);
     else if (string_equal(type, "u16"))
          vulkan_shader_add_uniform_u16(shader, name, scope, out_location);
     else if (string_equal(type, "u32"))
          vulkan_shader_add_uniform_u32(shader, name, scope, out_location);
     else if (string_equal(type, "u8"))
          vulkan_shader_add_uniform_u8(shader, name, scope, out_location);
     else if (string_equal(type, "f32"))
          vulkan_shader_add_uniform_f32(shader, name, scope, out_location);
     else if (string_equal(type, "vec2"))
          vulkan_shader_add_uniform_vec2(shader, name, scope, out_location);
     else if (string_equal(type, "vec3"))
          vulkan_shader_add_uniform_vec3(shader, name, scope, out_location);
}

void set_uniform(vulkan_shader *shader, const char *type, u32 location, void*value)
{
     if (string_equal(type, "mat4"))
     {
          vulkan_shader_set_uniform_mat4(shader, location, value);
     }
     else if (string_equal(type, "vec4"))
     {
          vulkan_shader_set_uniform_vec4(shader, location, value);
     }
     else if (string_equal(type, "vec3"))
     {
          vulkan_shader_set_uniform_vec3(shader, location, value);
     }
     else if (string_equal(type, "vec2"))
     {
          vulkan_shader_set_uniform_vec2(shader, location, value);
     }
     else if (string_equal(type, "i16"))
     {
          i16 *internal_value = (i16 *)value;
          vulkan_shader_set_uniform_i16(shader, location, *internal_value);
     }
     else if (string_equal(type, "i32"))
     {
          vulkan_shader_set_uniform_i32(shader, location, value);
     }
     else if (string_equal(type, "u8"))
     {
          u8 *internal_value = (u8 *)value;
          vulkan_shader_set_uniform_u8(shader, location, *internal_value);
     }
     else if (string_equal(type, "u16"))
     {
          u16 *internal_value = (u16 *)value;
          vulkan_shader_set_uniform_u16(shader, location, *internal_value);
     }
     else if (string_equal(type, "u32"))
     {
          u32 *internal_value = (u32 *)value;
          vulkan_shader_set_uniform_u32(shader, location, *internal_value);
     }
     else if (string_equal(type, "f32"))
     {
          
          vulkan_shader_set_uniform_f32(shader, location, value);
     }
}

void add_shader_property(const char *property_name, material_shader_config config, vulkan_shader *shader, material_shader *material_shader)
{
     u32 count = 0;
     if (string_equal(property_name, "attributes"))
     {
          count = config.attribute_count;
     }
     else if (string_equal(property_name, "uniforms"))
     {
          count = config.uniform_count;
     }
     else if (string_equal(property_name, "samplers"))
     {
          count = config.sampler_count;
     }
     for (u32 i = 0; i < count; i++)
     {
          if (string_equal(property_name, "attributes"))
          {
               vulkan_shader_add_attributes(shader, config.attribute_names[i], change_str_to_enum(config.attribute_type[i]));
          }
          else if (string_equal(property_name, "uniforms"))
          {
               u32 temp = 0;
               if (string_equal(config.uniform_scope[i], "GLOBAL"))
               {
                    ADD_UNIFORM(shader, SHADER_SCOPE_GLOBAL, config.uniform_type[i], config.uniform_names[i], &temp);
               }
               else if (string_equal(config.uniform_scope[i], "LOCAL"))
               {
                    ADD_UNIFORM(shader, SHADER_SCOPE_LOCAL, config.uniform_type[i], config.uniform_names[i], &temp);
               }
               else
               {
                    ADD_UNIFORM(shader, SHADER_SCOPE_INSTANCE, config.uniform_type[i], config.uniform_names[i], &temp);
               }
          }
          else if (string_equal(property_name, "samplers"))
          {
               u32 temp = 0;
               if (string_equal(config.sampler_scope[i], "GLOBAL"))
               {
                    vulkan_shader_add_sampler(shader, config.sampler_name[i], SHADER_SCOPE_GLOBAL, &temp);
               }
               else if (string_equal(config.uniform_scope[i], "LOCAL"))
               {
                    vulkan_shader_add_sampler(shader, config.sampler_name[i], SHADER_SCOPE_LOCAL, &temp);
               }
               else
               {
                    vulkan_shader_add_sampler(shader, config.sampler_name[i], SHADER_SCOPE_INSTANCE, &temp);
               }
          }
     }
}
shader_attribute_type change_str_to_enum(const char *str)
{
     shader_attribute_type type;
     for (u32 i = 0; i < 28; i++)
     {
          if (string_equal(context.array[i].str_variant, str))
          {
               type = context.array[i].value;
               break;
          }
     }
     return type;
}
bool vulkan_acquire_material_shader_resources(struct material_shader *shader, u32 *material_id)
{
     vulkan_shader *internal_data = (vulkan_shader *)shader->internal_data;
     return vulkan_shader_acquire_instance_resources(&context.shaders[internal_data->internal_id], material_id);
}
bool vulkan_release_material_shader_resources(struct material_shader *shader, u32 material_id)
{
     vulkan_shader *internal_data = (vulkan_shader *)shader->internal_data;
     if (vulkan_shader_release_instance_resources(&context.shaders[internal_data->internal_id], material_id))
     {
     };
     if (vulkan_shader_destroy(&context.shaders[internal_data->internal_id]))
     {
          u32 location = internal_data->internal_id;
          Dzero_memory(&context.shaders[internal_data->internal_id], sizeof(vulkan_shader));
          context.shaders[location].internal_id = INVALID_ID;
          return true;
     }
     return false;
};
