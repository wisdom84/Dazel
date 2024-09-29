#include "vulkan_framebuffer.h"
#include "core/Dmemory.h"


void vulkan_framebuffer_create(
    vulkan_context*context,
    u64 width,
    u64 height,
    u64 attachment_count,
    VkImageView* attachment,
    vulkan_renderpass * renderpass,
    vulkan_frame_buffers*out_framebuffer
){
     out_framebuffer->attachment = (VkImageView*)Dallocate_memory(sizeof(VkImageView)*attachment_count, MEMORY_TAG_RENDERER);
     for (u64 i = 0; i < attachment_count; i++)
     {
        out_framebuffer->attachment[i] = attachment[i];
     }
     out_framebuffer->renderpass = renderpass;
     out_framebuffer->attachment_count = attachment_count;

     //create info
         VkFramebufferCreateInfo frame_buffer_create_info; 
         frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
         frame_buffer_create_info.layers = 1;
         frame_buffer_create_info.renderPass = out_framebuffer->renderpass->handle;
         frame_buffer_create_info.height= height;
         frame_buffer_create_info.width = width;
         frame_buffer_create_info.pNext = nullptr;
         frame_buffer_create_info.attachmentCount = out_framebuffer->attachment_count;
         frame_buffer_create_info.pAttachments =out_framebuffer->attachment;
         frame_buffer_create_info.flags =0;
VK_CHECK(vkCreateFramebuffer(context->device.logical_device,
                            &frame_buffer_create_info,
                            context->allocator,
                            &out_framebuffer->handle));    
     
};

void vulkan_framebuffer_destroy(vulkan_context*context, vulkan_frame_buffers*framebuffer){
  vkDestroyFramebuffer(context->device.logical_device, framebuffer->handle,context->allocator);
  if(framebuffer->attachment){
       Dfree_memory(framebuffer->attachment,MEMORY_TAG_RENDERER,sizeof(VkImageView)*framebuffer->attachment_count);
       framebuffer->attachment = 0;
  }
  framebuffer->handle = 0;
  framebuffer->attachment_count = 0;
  framebuffer->renderpass = 0;
};