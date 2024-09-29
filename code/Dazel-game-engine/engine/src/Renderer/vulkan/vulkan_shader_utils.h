#pragma once 
#include "vulkan_types.inl"



bool create_shader_stage(vulkan_context*context,const char* shader_ext,
                          VkShaderStageFlagBits shader_stage_flag, u32 stage_index,
                          vulkan_shader_stage*shader_stage 
);