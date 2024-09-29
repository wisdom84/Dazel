#include "vulkan_command_buffer.h"
#include "core/Dmemory.h"
void vulkan_command_buffer_allocate(
    vulkan_context*context,
    VkCommandPool pool,
    bool is_primary,
    vulkan_command_buffer*out_command_buffer
){
  Dzero_memory(out_command_buffer, sizeof(out_command_buffer));  
 VkCommandBufferAllocateInfo command_buffer_allocate_info;
 command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
 command_buffer_allocate_info.commandBufferCount = 1;
 command_buffer_allocate_info.commandPool = pool;
 command_buffer_allocate_info.level = is_primary? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
 command_buffer_allocate_info.pNext = nullptr;
  out_command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
  VK_CHECK(vkAllocateCommandBuffers(context->device.logical_device, &command_buffer_allocate_info, &out_command_buffer->handle));
  out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
};

void vulkan_command_buffer_free(
    vulkan_context*context,
    VkCommandPool pool,
    vulkan_command_buffer*command_buffer
){
    vkFreeCommandBuffers(
        context->device.logical_device,
        pool,
        1,
        &command_buffer->handle
    );
    command_buffer->handle = 0;
    command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED; 
};
void vulkan_command_buffer_begin(
    vulkan_command_buffer * command_buffer,
    bool is_single_use,
    bool is_renderpass_continue,
    bool is_simultaneous_use
){
 VkCommandBufferBeginInfo command_buffer_begin_info;
command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
command_buffer_begin_info.flags =  0;
if(is_single_use){
   command_buffer_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  
}
if(is_renderpass_continue){
    command_buffer_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
}
if(is_simultaneous_use){
    command_buffer_begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
}
command_buffer_begin_info.pNext =nullptr;
command_buffer_begin_info.pInheritanceInfo = nullptr;
 vkBeginCommandBuffer(command_buffer->handle,&command_buffer_begin_info);
command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
};

void vulkan_command_buffer_end(vulkan_command_buffer*command_buffer){
 vkEndCommandBuffer(command_buffer->handle);
 command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
};
void vulakn_command_buffer_update_submitted(vulkan_command_buffer*command_buffer){
  command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
};
void vulkan_command_buffer_reset(vulkan_command_buffer*command_buffer){
   vkResetCommandBuffer(command_buffer->handle, 0);
   command_buffer->state = COMMAND_BUFFER_STATE_READY;
};

void vulkan_command_buffer_allocate_and_begin_single_use(
    vulkan_context*context,
    VkCommandPool pool,
    vulkan_command_buffer*out_command_buffer
){
   vulkan_command_buffer_allocate(context,pool,true,out_command_buffer);
   vulkan_command_buffer_begin(out_command_buffer,true,false,false);
};

void vulkan_command_buffer_end_single_use(
    vulkan_context*context,
    VkCommandPool pool,
    vulkan_command_buffer*out_command_buffer,
    VkQueue queue
){
   vulkan_command_buffer_end(out_command_buffer);
   VkSubmitInfo Queue_submit_info;
   Queue_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   Queue_submit_info.commandBufferCount = 1;
   Queue_submit_info.pCommandBuffers = &out_command_buffer->handle;
   Queue_submit_info.pNext = nullptr;
   Queue_submit_info.waitSemaphoreCount = 0;
   Queue_submit_info.pWaitSemaphores =nullptr;
   Queue_submit_info.pWaitDstStageMask = nullptr;
   Queue_submit_info.signalSemaphoreCount = 0;
   Queue_submit_info.pSignalSemaphores =nullptr;
   vkQueueSubmit(queue,1,&Queue_submit_info, VK_NULL_HANDLE);
   vkQueueWaitIdle(queue);
   vulkan_command_buffer_free(context,pool,out_command_buffer);
};