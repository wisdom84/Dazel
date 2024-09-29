#pragma once 
#include "Renderer/vulkan/vulkan_types.inl"
#include "resources/resources.inl"
// #include"Renderer/Renderer_global_states.h"
#include "core/logger.h"

bool vulkan_shader_object_UI_create(vulkan_context*context,vulkan_shader_UI_object*out_shader);

void vulkan_shader_object_UI_destroy(vulkan_context*context, vulkan_shader_UI_object*out_shader);

void vulkan_object_shader_UI_use(vulkan_context*context,vulkan_shader_UI_object*shader);

void vulkan_shader_object_update_global_UI_state(vulkan_context*context, vulkan_shader_UI_object*shader,float delta_time);

void vulkan_object_shader_UI_set_model(vulkan_context*context,  vulkan_shader_UI_object*shader, mat4 model);

void vulkan_object_shader_UI_apply_material(vulkan_context*context,  vulkan_shader_UI_object*shader, materials*material);


bool vulkan_shader_object_UI_aquire_resources(vulkan_context*context,struct vulkan_shader_UI_object*shader, materials*material);

void vulkan_shader_object_UI_release_resources(vulkan_context*context, struct vulkan_shader_UI_object*shader,materials*material);