#include "vulkan_buffer.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"
#include "core/logger.h"
#include "core/Dmemory.h"

void cleanup_freelist(vulkan_buffer*buffer){
    freelist_destroy(&buffer->buffer_freelist);
    Dfree_memory(buffer->freelist_block, MEMORY_TAG_RENDERER, buffer->freelist_memory_requirement);
    buffer->freelist_memory_requirement = 0;
    buffer->freelist_block = nullptr;
}

bool vulkan_bind_buffer_memory(vulkan_context*context, vulkan_buffer*buffer, u64 offset){
 VkResult result =vkBindBufferMemory(context->device.logical_device,buffer->handle, buffer->memory,offset);
   if(vulkan_result_is_success(result)){
    DINFO(" buffer memory  was  binded  succcessfully");
    return true;
   }  
DERROR("vkBindBufferMemory failed with message: %s.",vulkan_result_string(result, true));
return false;
}

bool vulkan_buffer_create(
    vulkan_context*context,
    VkDeviceSize size,
    VkMemoryPropertyFlags mem_prop_flags,
    bool bind_on_create,
    VkBufferUsageFlags buffer_usage,
    vulkan_buffer*out_buffer
){
   out_buffer->size = size;
   out_buffer->buffer_usage = buffer_usage;
   out_buffer->memory_flags = mem_prop_flags;

   out_buffer->freelist_memory_requirement = 0;
   freelist_create(size,&out_buffer->freelist_memory_requirement,0,0);
   out_buffer->freelist_block = Dallocate_memory(out_buffer->freelist_memory_requirement,MEMORY_TAG_RENDERER);
   freelist_create(size,&out_buffer->freelist_memory_requirement,out_buffer->freelist_block, &out_buffer->buffer_freelist);

     VkBufferCreateInfo buffer_create_info;
     buffer_create_info.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
     buffer_create_info.pNext = nullptr;
     buffer_create_info.flags = 0;
     buffer_create_info.pQueueFamilyIndices = nullptr;
     buffer_create_info.queueFamilyIndexCount =0;
     buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
     buffer_create_info.usage = buffer_usage;
     buffer_create_info.size = size;
 VK_CHECK(vkCreateBuffer(context->device.logical_device, 
                        &buffer_create_info,
                        nullptr,
                        &out_buffer->handle)); 
    // gather memory requirements for the buffer
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logical_device, out_buffer->handle, &requirements);
    out_buffer->memory_index =  context->find_memory_index(requirements.memoryTypeBits, out_buffer->memory_flags);
    if(out_buffer->memory_index == -1){
        DERROR("unable to create vulkan buffer becuase memory requirements where not found");
        return false;
        cleanup_freelist(out_buffer);
    }
    // Allocate memory
  VkMemoryAllocateInfo mem_allocate_info;
  mem_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_allocate_info.memoryTypeIndex = out_buffer->memory_index;
  mem_allocate_info.allocationSize =requirements.size;
  mem_allocate_info.pNext = nullptr;
  VkResult result = vkAllocateMemory(context->device.logical_device,
                                    &mem_allocate_info, 
                                    nullptr,
                                    &out_buffer->memory);  
   if(vulkan_result_is_success(result)){
    DINFO("buffer memory was allocated succcessfully");
   }  
   else{
    DERROR("vkAllocateMemory failed with message: %s.",vulkan_result_string(result, true));
    return false;
    cleanup_freelist(out_buffer);
   } 
   // bind memory
   if( bind_on_create){
     if(!vulkan_bind_buffer_memory(context,out_buffer,0)){
        DERROR("failed to bind memory");
        return false;
        cleanup_freelist(out_buffer);
     };
   } 
 return true;
};

void vulkan_buffer_destroy(
    vulkan_context*context,
    vulkan_buffer*buffer
){
    if(buffer->freelist_block){
       cleanup_freelist(buffer);
    }

    if(buffer){
            if(buffer->handle){
                vkDestroyBuffer(context->device.logical_device,buffer->handle,context->allocator);
                buffer->handle=0;
                if(buffer->memory){
                    vkFreeMemory(context->device.logical_device,buffer->memory,context->allocator);
                    buffer->memory= 0;
                }
            }
        
    }
    buffer->size = 0;
    buffer->memory_flags = 0;
    buffer->memory_index = -1;
    buffer->is_locked = false;

};

void* buffer_map_memory(vulkan_context*context,vulkan_buffer*buffer,u64 offset, u64 size, VkMemoryPropertyFlags flags){
    void*data;
    VK_CHECK(vkMapMemory(context->device.logical_device,buffer->memory,offset,size,flags,&data));
    return data;
};
void buffer_unmap_memory(vulkan_context*context,vulkan_buffer*buffer){
   vkUnmapMemory(context->device.logical_device,buffer->memory);
};
void vulkan_buffer_load_data(
    vulkan_context*context,
    vulkan_buffer*buffer,
    u64 offset,
     u64 size, 
     VkMemoryPropertyFlags flags,
     const void*data
){
  void*mapped_memory = buffer_map_memory(context,buffer,offset,size,flags);
  Dmemory_copy(mapped_memory, data,size);
  buffer_unmap_memory(context,buffer);
};

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
){ 
    vulkan_command_buffer command_buffer;
    vulkan_command_buffer_allocate_and_begin_single_use(context,pool,&command_buffer);
    // region for copy
    VkBufferCopy copy_region;
    copy_region.dstOffset = dest_offset;
    copy_region.size = size;
    copy_region.srcOffset = source_offset;
    vkCmdCopyBuffer(command_buffer.handle,*source,*dest,1,&copy_region);
    vulkan_command_buffer_end_single_use(context,pool,&command_buffer,queue);
};

bool vulkan_buffer_resize(
    vulkan_context*context,
    u64 new_size,
    vulkan_buffer*buffer,
    VkQueue queue,
    VkCommandPool pool
){
   // freelist resizing and allocation
    if(new_size < buffer->size){
        DERROR("vulkan_buffer resize requires that the new size be larger or equal to the previous size");
        return false;
    }
    u64 new_memory_requirement = 0;
    freelist_resize(&buffer->buffer_freelist, &new_memory_requirement, 0,0,0);
    void*new_block = Dallocate_memory(new_memory_requirement, MEMORY_TAG_RENDERER);
    void*old_block = 0;
    if(!freelist_resize(&buffer->buffer_freelist, &new_memory_requirement, new_block, new_size,&old_block)){
        DERROR("vulkan_buffer_resize failed to resize internal free list");
        Dfree_memory(new_block, MEMORY_TAG_RENDERER,new_memory_requirement);
        return false;
    }
    Dfree_memory(old_block, MEMORY_TAG_RENDERER, buffer->freelist_memory_requirement);
    buffer->freelist_memory_requirement = new_memory_requirement;
    buffer->freelist_block = new_block;
    // buffer->size = new_size;

    //  buffer->size = new_size;
     vulkan_buffer new_buffer;
     VkBufferCreateInfo buffer_create_info;
     buffer_create_info.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
     buffer_create_info.pNext = nullptr;
     buffer_create_info.flags = 0;
     buffer_create_info.pQueueFamilyIndices = nullptr;
     buffer_create_info.queueFamilyIndexCount =0;
     buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
     buffer_create_info.usage = buffer->buffer_usage;
     buffer_create_info.size = new_size;
 VK_CHECK(vkCreateBuffer(context->device.logical_device, 
                        &buffer_create_info,
                        nullptr,
                        &new_buffer.handle)); 
    // gather memory requirements for the buffer
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logical_device, new_buffer.handle, &requirements);
    buffer->memory_index =  context->find_memory_index(requirements.memoryTypeBits, buffer->memory_flags);
    if(buffer->memory_index == -1){
        DERROR("unable to resize  vulkan buffer becuase memory requirements where not found");
        return false;
    }
    // Allocate memory

  VkMemoryAllocateInfo mem_allocate_info;
  mem_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_allocate_info.memoryTypeIndex = buffer->memory_index;
  mem_allocate_info.allocationSize =requirements.size;
  mem_allocate_info.pNext = nullptr;
  VkResult result = vkAllocateMemory(context->device.logical_device,
                                    &mem_allocate_info, 
                                    nullptr,
                                    &new_buffer.memory);  
   if(vulkan_result_is_success(result)){
    DINFO("buffer memory was allocated succcessfully");
   }  
   else{
    DERROR("vkAllocateMemory failed with message: %s.",vulkan_result_string(result, true));
    return false;
   } 
   // bind memory
     if(!vulkan_bind_buffer_memory(context,&new_buffer,0)){
        DERROR("failed to bind memory");
        return false;
     };
  vulkan_buffer_copy(context,pool,VK_NULL_HANDLE,queue,&buffer->handle,0,&new_buffer.handle,0,buffer->size);
  // Destroy the old buffer
    if(buffer){
            if(buffer->handle){
                vkDestroyBuffer(context->device.logical_device,buffer->handle,context->allocator);
                buffer->handle=0;
                if(buffer->memory){
                    vkFreeMemory(context->device.logical_device,buffer->memory,context->allocator);
                    buffer->memory= 0;
                }
            }
    }
    buffer->handle= new_buffer.handle;
    buffer->memory = new_buffer.memory;
    buffer->size = new_size;
 return true;
};
bool vulkan_buffer_allocate(vulkan_buffer*buffer, u64 size, u64* out_offset){
    if(!buffer || !size || !out_offset){
         DERROR("vulkan_buffer allocate requrires a valid buffer and size to allocate memory for");
         return false;
    }
    return freelist_allocate_block(&buffer->buffer_freelist, size, out_offset);
};

bool vulkan_buffer_free(vulkan_buffer*buffer, u64 size, u64 offset){
    if(!buffer || !size){
         DERROR("vulkan_buffer free requrires a valid buffer and size to free memory for");
         return false;
    }
    return freelist_free_block(&buffer->buffer_freelist, size, offset);
};


