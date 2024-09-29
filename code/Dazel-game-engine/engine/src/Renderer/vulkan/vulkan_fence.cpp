#include "vulkan_fence.h"

#include "core/logger.h"

void vulkan_fence_create(
    vulkan_context*context,
    bool create_signaled,
    vulkan_fence* out_fence
){
    out_fence->is_signaled = create_signaled;
    VkFenceCreateInfo fence_create_info={VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if(out_fence->is_signaled){
         fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }
    else{
       fence_create_info.flags=0;
    }
    fence_create_info.pNext = nullptr;
    VK_CHECK(vkCreateFence(context->device.logical_device, &fence_create_info, context->allocator, &out_fence->handle));
};
void vulkan_fence_destroy(vulkan_context* context, vulkan_fence* fence){
  if(fence->handle){
    vkDestroyFence(
        context->device.logical_device,
        fence->handle,
        context->allocator
    );
    fence->handle = 0;

  }
  fence->is_signaled = false;
};

bool vulkan_fence_wait(vulkan_context* context, vulkan_fence* fence, u64 timeout_ns){
   if(!fence->is_signaled){
     VkResult result = vkWaitForFences(context->device.logical_device,1,&fence->handle, true, timeout_ns);
     switch(result){
        case VK_SUCCESS:
            fence->is_signaled = true;
            return true;
        case VK_TIMEOUT:
             DWARNING("vk fence timed out");
             break;
        case VK_ERROR_DEVICE_LOST:
            DERROR("vk_fence_wait:VK_ERROR_DEVICE_LOST");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
             DERROR("vk_fence_wait: VK_ERROR_OUT_OF_HOST_MEMORY");
             break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            DERROR("vk_fence_wait:  VK_ERROR_OUT_OF_DEVICE_MEMORY");
            break; 
        default:
             DERROR("vk_fence_wait: An unknown error has occured"); 
             break;                   
     }
   }else{
    // if already signaled do not call
    return true;
   }
   return false;
};

void vulkan_fence_reset(vulkan_context*context, vulkan_fence*fence){
   if(fence->is_signaled){
    VK_CHECK(vkResetFences(context->device.logical_device,1,&fence->handle));
    fence->is_signaled = false;
   }
};