#pragma once
#include "vulkan_types.inl"

bool vulkan_graphics_pipeline_create(
    vulkan_context*context,
    vulkan_renderpass*renderpass,
    u32 stride,
    u32 attribute_count,
    VkVertexInputAttributeDescription*attribute_description,
    VkDescriptorSetLayout*descriptor_set_layout,
    u64 descriptor_set_layout_count,
    u32 stage_count,
    VkPipelineShaderStageCreateInfo*shader_stages,
    VkViewport viewport,
     VkRect2D scissor,
    bool is_wireframe,
    bool depth_test_enabled,
    vulkan_pipeline*out_pipeline
);

void vulkan_graphics_pipeline_destroy( vulkan_context*context,vulkan_pipeline*pipeline);

void vulkan_bind_pipeline(vulkan_command_buffer*command_buffer, VkPipelineBindPoint bind_point,vulkan_pipeline*pipeline);
 