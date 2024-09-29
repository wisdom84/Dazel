#pragma once 
#include "Renderer/vulkan/vulkan_types.inl"
#include "resources/resources.inl"
// #include"Renderer/Renderer_global_states.h"
#include "core/logger.h"

bool vulkan_shader_object_create(vulkan_context*context, vulkan_renderpass*renderpass,vulkan_shader_object*out_shader,const char*shader_file_name);

void vulkan_shader_object_destroy(vulkan_context*context, vulkan_shader_object*out_shader);

void vulkan_object_shader_use(vulkan_context*context,vulkan_shader_object*shader);

void vulkan_shader_object_update_global_state(vulkan_context*context, vulkan_shader_object*shader,float delta_time);

void vulkan_object_shader_set_model(vulkan_context*context,  vulkan_shader_object*shader, mat4 model);

void vulkan_object_shader_apply_material(vulkan_context*context,  vulkan_shader_object*shader, materials*material, bool apply_material);


bool vulkan_shader_object_aquire_resources(vulkan_context*context,struct vulkan_shader_object*shader, materials*material);

void vulkan_shader_object_release_resources(vulkan_context*context, struct vulkan_shader_object*shader,materials*material);

