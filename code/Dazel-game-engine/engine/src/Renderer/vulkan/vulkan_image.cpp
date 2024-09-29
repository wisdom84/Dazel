#include "vulkan_image.h"
#include "vulkan_command_buffer.h"
#include "core/Dmemory.h"

#include "core/logger.h"
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
){
    //copy param
    out_image->height = height;
    out_image->width  = width;
    //create info
    VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_create_info.flags =0;
    image_create_info.pNext = nullptr;
    image_create_info.usage = usage_flag;
    image_create_info.tiling = tiling;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers= 1;
    image_create_info.pQueueFamilyIndices = nullptr;
    image_create_info.queueFamilyIndexCount = 0;
    VK_CHECK(vkCreateImage(context->device.logical_device, &image_create_info,context->allocator, &out_image->handle));
    //query memory requirements for the image
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(context->device.logical_device,out_image->handle,&mem_requirements);
    int memory_type = context->find_memory_index(mem_requirements.memoryTypeBits,memory_property);
    if(memory_type == -1){
        DERROR("Required memory type not found. Image not valid");
    }
    //Allocate memory
    VkMemoryAllocateInfo mem_allocate;
    mem_allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_allocate.allocationSize = mem_requirements.size;
    mem_allocate.memoryTypeIndex =memory_type;
    mem_allocate.pNext = nullptr;
   VK_CHECK(vkAllocateMemory(context->device.logical_device,&mem_allocate,context->allocator,&out_image->memory));
   //bind memory
   VK_CHECK(vkBindImageMemory(context->device.logical_device,out_image->handle,out_image->memory,0));
   // create view
    out_image->view = 0;
    vulkan_image_view_create(context,format, out_image, image_aspect_flag);
 
};
void vulkan_image_view_create(
    vulkan_context*context,
    VkFormat format,
    vulkan_image*image,
    VkImageAspectFlags aspect_flags
){
    VkImageViewCreateInfo  view_info=  {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.flags = 0;
    view_info.pNext = nullptr;
    view_info.image = image->handle;
    view_info.format =format;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.subresourceRange.aspectMask =aspect_flags;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.subresourceRange.levelCount =1;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    VK_CHECK(vkCreateImageView(context->device.logical_device,&view_info,context->allocator,&image->view));
};
void vulkan_image_destroy(vulkan_context*context,vulkan_image*image){
    if(image->view){
        vkDestroyImageView(context->device.logical_device, image->view, context->allocator); 
        image->view = 0;  
    }   
    if(image->memory){
        vkFreeMemory(context->device.logical_device, image->memory, context->allocator);
        image->memory = 0;
    }
    if(image->handle){
      vkDestroyImage(context->device.logical_device, image->handle, context->allocator); 
      image->handle = 0;
    }
};

void vulkan_image_transition_layout(
    vulkan_context*context,
    vulkan_command_buffer*command_buffer,
    vulkan_image*image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
){
    VkImageMemoryBarrier image_memory_barrier; // pipeline barrier
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.image = image->handle;
    image_memory_barrier.oldLayout = old_layout;
    image_memory_barrier.newLayout = new_layout;
    image_memory_barrier.pNext = nullptr;
    image_memory_barrier.srcQueueFamilyIndex = context->device.graphics_queue_family_index; // expands to zero
    image_memory_barrier.dstQueueFamilyIndex = context->device.graphics_queue_family_index; // expands to zero
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0; 
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;
    image_memory_barrier.subresourceRange.levelCount = 1;
    VkPipelineStageFlags srcStageFlag;
    VkPipelineStageFlags dstStageFlag;
    if(old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
        image_memory_barrier.srcAccessMask = 0;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStageFlag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
   else if(old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
        image_memory_barrier.srcAccessMask =  VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;
        srcStageFlag = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStageFlag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else{
        DFATAL("Unsupported image layout transition");
        return;
    }
   vkCmdPipelineBarrier(command_buffer->handle,srcStageFlag,dstStageFlag,0,0,nullptr,0,nullptr,1,&image_memory_barrier);
};

void vulkan_image_copy_from_buffer(
    vulkan_context *context,
    vulkan_image*image,
    vulkan_buffer*buffer,
    vulkan_command_buffer*command_buffer
){
  VkBufferImageCopy buffer_image_copy_region;
  buffer_image_copy_region.bufferOffset = 0;
  buffer_image_copy_region.imageExtent.width = (uint32_t)image->width;
  buffer_image_copy_region.imageExtent.height = (uint32_t)image->height;
  buffer_image_copy_region.imageExtent.depth = 1;
  buffer_image_copy_region.bufferRowLength = 0; // the Row length to calculate data spacing 
  buffer_image_copy_region.bufferImageHeight = 0; // the height to calculate data spacing 
  buffer_image_copy_region.imageOffset = {0,0,0};
  buffer_image_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  buffer_image_copy_region.imageSubresource.baseArrayLayer = 0; 
  buffer_image_copy_region.imageSubresource.mipLevel = 0;
  buffer_image_copy_region.imageSubresource.layerCount = 1;
  vkCmdCopyBufferToImage(command_buffer->handle, buffer->handle, image->handle,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,&buffer_image_copy_region);
};
 