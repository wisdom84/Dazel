#pragma once
#include "vulkan_types.inl"

enum renderpass_clear_flags{
   RENDERPASS_CLEAR_NONE_FLAG = 0x0,
   RENDERPASS_CLEAR_COLOR_BUFFER_FLAG = 0x1,
   RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG = 0x2,
   RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4
};
void vulkan_renderpass_create(
     vulkan_context*context,
     vulkan_renderpass*out_renderpass,
     float x, float y, float w, float h,
     float r, float g, float b, float a,
     float depth,
     u64 stencil,
     u8 clear_flag,
     bool has_prev_pass,
     bool has_next_pass
);

void vulkan_renderpass_destroy(vulkan_context*context, vulkan_renderpass* renderpass);

void vulkan_renderpass_begin(
    vulkan_command_buffer*command_buffer,
    vulkan_renderpass* renderpass,
     VkFramebuffer frame_buffer
);

void vulkan_renderpass_end(vulkan_command_buffer*command_buffer, vulkan_renderpass*renderpass);