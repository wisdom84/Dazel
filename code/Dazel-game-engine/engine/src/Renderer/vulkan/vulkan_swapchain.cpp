#include "core/logger.h"
#include "core/Dmemory.h"
#include "vulkan_device.h"
#include "vulkan_image.h"
float Clamp(u64 value, u64 min_num, u64 max_num){
  return value <= min_num ? min_num : value >= max_num ? max_num : value;
}
void create(vulkan_context*context, u64*width, u64*height, vulkan_swapchain*swapchain);
void destroy(vulkan_context*context, vulkan_swapchain* swapchain);


void vulkan_swapchain_create(
     vulkan_context*context,
     u64 *width,
     u64 *height,
     vulkan_swapchain*out_swapchain
){
  create(context, width, height,out_swapchain);
};

void vulkan_swapchain_recreate(
     vulkan_context*context,
     u64 *width,
     u64 *height,
     vulkan_swapchain*out_swapchain
){
  destroy(context,out_swapchain);
  create(context, width, height, out_swapchain);
};
void vulkan_swapchain_destroy(
    vulkan_context*context,
    vulkan_swapchain*swapchain
){
    destroy(context, swapchain);
};




bool vulkan_swapchain_aquire_next_image_index(
    vulkan_context*context,
    vulkan_swapchain*swapchain,
    u64 timeout_ns,
    VkSemaphore image_available_semaphore,
    VkFence fence,
    u32*out_image_index
){
  VkResult result =vkAcquireNextImageKHR(context->device.logical_device, 
                         swapchain->handle,
                         timeout_ns,
                         image_available_semaphore,
                         fence,
                         out_image_index);
    if(result == VK_ERROR_OUT_OF_DATE_KHR){
        // trigger swapchain recreation most likely when the window is on resize
        vulkan_swapchain_recreate(context,&context->framebuffer_width,
                                 &context->framebuffer_height, swapchain);
          return false;                       
    }  
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
        DFATAL("failed to aquire swapchain image");
        return false;
    } 
    return true;                  
};





bool vulkan_swapchain_present(
    vulkan_context*context,
    vulkan_swapchain*swapchain,
    VkQueue graphics_queue,
    VkQueue present_queue,
    VkSemaphore render_available_semaphore,
    u32*out_present_image_index
){
   VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_available_semaphore;
    present_info.pSwapchains = &swapchain->handle;
    present_info.pImageIndices =(const uint32_t*)out_present_image_index;
    present_info.pResults = nullptr;
    present_info.swapchainCount = 1;
    present_info.pNext = nullptr;

    VkResult result = vkQueuePresentKHR(present_queue,&present_info);
      if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR){
        // trigger swapchain recreation most likely when the window is on resize
        vulkan_swapchain_recreate(context,&context->framebuffer_width,
                                 &context->framebuffer_height, swapchain);
        //   return false;                       
    }  
    else if(result != VK_SUCCESS){
        DFATAL("failed to present swapchain image");
        // return false;
    } 
  context->current_frame = (context->current_frame+1) % swapchain->max_frames_in_flight;  
  return true;
};

void create(vulkan_context*context, u64*width, u64*height, vulkan_swapchain*swapchain){
  uint32_t width_t = (uint32_t)*width;
  uint32_t height_t = (uint32_t)*height;
  VkExtent2D swapchain_extent= {width_t,height_t};
  swapchain->max_frames_in_flight = 2; // support tripple buffering 
  // choose a surface format
  bool found = false;
  for(u64 i=0; i < context->device.swapchain_support.format_count; i++){
    VkSurfaceFormatKHR format = context->device.swapchain_support.surface_format[i];
    // prefered format
    if(format.format == VK_FORMAT_B8G8R8A8_UNORM &&
       format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
         swapchain->image_format = format;
         found = true;
         break;
       }
  }
  if(!found){
    swapchain->image_format = context->device.swapchain_support.surface_format[0];
  }

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for(u64 j=0; j < context->device.swapchain_support.present_mode_count; j++){
     VkPresentModeKHR mode = context->device.swapchain_support.present_mode[j];
     if(mode == VK_PRESENT_MODE_MAILBOX_KHR){
        present_mode = mode;
        break;
     }
  }
  // requery swapchain support
  vulkan_device_query_swapchain_support(
           context->device.physical_device,
           context->surface,
           &context->device.swapchain_support
        );

    // swapchain extent 
    if(context->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX){
        swapchain_extent = context->device.swapchain_support.capabilities.currentExtent;
        context->swapchain.extent.width = swapchain_extent.width;
        context->swapchain.extent.height = swapchain_extent.height;
    } 
    // clamp to the value allowed by the GPU 
    else{
          swapchain_extent.width =  Clamp(swapchain_extent.width, context->device.swapchain_support.capabilities.minImageExtent.width,context->device.swapchain_support.capabilities.maxImageExtent.width);
          swapchain_extent.height = Clamp(swapchain_extent.height, context->device.swapchain_support.capabilities.minImageExtent.height,context->device.swapchain_support.capabilities.maxImageExtent.height);
         context->swapchain.extent.width = swapchain_extent.width;
         context->swapchain.extent.height = swapchain_extent.height;
    }

    // *width = swapchain_extent.width;
    // *height = swapchain_extent.height;
    u64 image_count = context->device.swapchain_support.capabilities.minImageCount +1;
    if(context->device.swapchain_support.capabilities.minImageCount > 0 && image_count > context->device.swapchain_support.capabilities.maxImageCount){
        image_count = context->device.swapchain_support.capabilities.maxImageCount;
    }
    // swapchain create info
    VkSwapchainCreateInfoKHR swapchain_create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchain_create_info.surface = context->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain->image_format.format;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageColorSpace = swapchain->image_format.colorSpace;
    swapchain_create_info.imageArrayLayers = 1;
    if(context->device.graphics_queue_family_index == context->device.present_queue_family_index){
      swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      swapchain_create_info.pQueueFamilyIndices = nullptr;
      swapchain_create_info.queueFamilyIndexCount = 0;
    }
    else{
        u64 queue_family_indicies[]={
            (u64)context->device.graphics_queue_family_index,
            (u64)context->device.present_queue_family_index
        };
      swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      swapchain_create_info.pQueueFamilyIndices =(const uint32_t*) queue_family_indicies;
      swapchain_create_info.queueFamilyIndexCount = 2;  
    }

    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.preTransform = context->device.swapchain_support.capabilities.currentTransform;
   
    swapchain_create_info.flags = 0;
    swapchain_create_info.clipped =VK_TRUE;
    swapchain_create_info.compositeAlpha= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.oldSwapchain = 0;
  VK_CHECK(vkCreateSwapchainKHR(context->device.logical_device, &swapchain_create_info,context->allocator,&swapchain->handle));
   //start with the current frame at zero
   context->current_frame = 0;
   context->image_index = 0;
   swapchain->image_count = 0;
   VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device,swapchain->handle,&swapchain->image_count,nullptr));
   if(!swapchain->images){
      swapchain->images =(VkImage*)Dallocate_memory(sizeof(VkImage)*swapchain->image_count, MEMORY_TAG_RENDERER);
   }
   if(!swapchain->image_views){
      swapchain->image_views =(VkImageView*)Dallocate_memory(sizeof(VkImageView)*swapchain->image_count, MEMORY_TAG_RENDERER);
   }
   VK_CHECK(vkGetSwapchainImagesKHR(context->device.logical_device,swapchain->handle,&swapchain->image_count,swapchain->images));
   // creat our views
   for(u64 j=0; j < swapchain->image_count; j++){
    VkImageViewCreateInfo  view_info=  {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.flags = 0;
    view_info.pNext = nullptr;
    view_info.image = swapchain->images[j];
    view_info.format = swapchain->image_format.format;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.subresourceRange.levelCount =1;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    VK_CHECK(vkCreateImageView(context->device.logical_device,&view_info,context->allocator,(swapchain->image_views+j)));
   }
  // Depth resources
  if(!vulkan_device_detect_depth_format(&context->device)){
    context->device.depth_format = VK_FORMAT_UNDEFINED;
    DFATAL("failed to find supported depth format");
  }
  vulkan_image_create(
    context,
    swapchain_extent.width,
    swapchain_extent.height,
    context->device.depth_format,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    VK_IMAGE_ASPECT_DEPTH_BIT,
    &context->swapchain.depth_attachment
  );
  DINFO("swapchain created successfully");

};
void destroy(vulkan_context*context, vulkan_swapchain* swapchain){
  vulkan_image_destroy(context,&swapchain->depth_attachment);

  for(u32 i=0; i < swapchain->image_count; i++){
    vkDestroyImageView(context->device.logical_device,context->swapchain.image_views[i],context->allocator);
  }
  vkDestroySwapchainKHR(context->device.logical_device,context->swapchain.handle,context->allocator);
};