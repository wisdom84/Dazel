#pragma once 
#include "vulkan_types.inl"

void vulkan_swapchain_create(
     vulkan_context*context,
     u64*width,
     u64*height,
     vulkan_swapchain*out_swapchain
);

void vulkan_swapchain_recreate(
     vulkan_context*context,
     u64 *width,
     u64 *height,
     vulkan_swapchain*out_swapchain
);

void vulkan_swapchain_destroy(
    vulkan_context*context,
    vulkan_swapchain*swapchain
);

bool vulkan_swapchain_aquire_next_image_index(
    vulkan_context*context,
    vulkan_swapchain*swapchain,
    u64 timeout_ns,
    VkSemaphore image_available_semaphore,
    VkFence fence,
     u32*out_image_index
);

bool vulkan_swapchain_present(
    vulkan_context*context,
    vulkan_swapchain*swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_available_semaphore,
    u32*out_present_image_index
);
