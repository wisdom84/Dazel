#pragma once
#include "vulkan_types.inl"

void vulkan_framebuffer_create(
    vulkan_context*context,
    u64 width,
    u64 height,
    u64 attachment_count,
    VkImageView* attachment,
    vulkan_renderpass * renderpass,
    vulkan_frame_buffers*out_framebuffer
);

void vulkan_framebuffer_destroy(vulkan_context*context, vulkan_frame_buffers*framebuffer);

