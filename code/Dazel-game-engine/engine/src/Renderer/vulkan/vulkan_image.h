#pragma once 
#include "vulkan_types.inl"
void vulkan_image_create(
    vulkan_context*context,
    u64 width,
    u64 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage_flag,
    VkMemoryPropertyFlags memory_property,
    VkImageAspectFlags image_aspect_flag,
    vulkan_image*out_image
);

void vulkan_image_transition_layout(
    vulkan_context*context,
    vulkan_command_buffer*command_buffer,
    vulkan_image*image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
);
void vulkan_image_copy_from_buffer(
    vulkan_context *context,
    vulkan_image*image,
    vulkan_buffer*buffer,
    vulkan_command_buffer*command_buffer
);

void vulkan_image_view_create(
    vulkan_context*context,
    VkFormat format,
    vulkan_image*image,
    VkImageAspectFlags aspect_flags
);

void vulkan_image_destroy(vulkan_context*context,vulkan_image*image);

