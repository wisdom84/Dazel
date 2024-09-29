#pragma once
#include "vulkan_types.inl"
bool vulkan_bind_buffer_memory(vulkan_context*context,vulkan_buffer*buffer, u64 offset);

bool vulkan_buffer_create(
    vulkan_context*context,
    VkDeviceSize size,
    VkMemoryPropertyFlags mem_prop_flags,
    bool bind_on_create,
    VkBufferUsageFlags buffer_usage,
    vulkan_buffer*out_buffer
);

void vulkan_buffer_destroy(
    vulkan_context*context,
    vulkan_buffer*buffer
);

void*buffer_map_memory(vulkan_context*context,vulkan_buffer*buffer,u64 offset, u64 size, VkMemoryPropertyFlags flags);
void buffer_unmap_memory(vulkan_context*context,vulkan_buffer*buffer);

void vulkan_buffer_load_data(
    vulkan_context*context,
    vulkan_buffer*buffer,
    u64 offset,
    u64 size, 
    VkMemoryPropertyFlags flags,
    const void*data
);

void vulkan_buffer_copy(
    vulkan_context*context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer*source,
    u64 source_offset,
    VkBuffer*dest,
    u64 dest_offset,
    u64 size
);

bool vulkan_buffer_resize(
    vulkan_context*context,
    u64 new_size,
    vulkan_buffer*buffer,
    VkQueue queue,
    VkCommandPool pool
);
bool vulkan_buffer_allocate(vulkan_buffer*buffer, u64 size, u64* out_offset);

bool vulkan_buffer_free(vulkan_buffer*buffer, u64 size, u64 offset);



