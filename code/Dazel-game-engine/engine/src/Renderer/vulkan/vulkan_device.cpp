
#include "vulkan_device.h"
#include "core/logger.h"
#include "core/Dstrings.h"
#include "core/Dmemory.h"
#include "containers/Darray.h"
typedef struct vulkan_physical_device_requirements{
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    const char**device_extension_names;
    bool sampler_anisotropy;
    bool integrated_gpu;
    bool discrete_gpu;
}vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info{
 u32 graphics_queue_family_index;
 u32 present_queue_family_index;
 u32 compute_queue_family_index;
 u32 transfer_queue_family_index;
}vulkan_physical_device_queue_family_info;


bool physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkPhysicalDeviceProperties* properties,
    VkPhysicalDeviceFeatures* features,
    vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info*out_queue_family_info,
    vulkan_swapchain_support_info*out_swapchain_support,
    VkSurfaceKHR surface);
bool select_physical_device(vulkan_context*context);

bool vulkan_device_create(vulkan_context*context){
  if(!select_physical_device(context)){
    return false;
  }
  DINFO("creating logical device");
  bool present_shares_graphics_queue = context->device.graphics_queue_family_index == context->device.present_queue_family_index;
  bool transfer_shares_graphics_queue = context->device.transfer_queue_family_index == context->device.graphics_queue_family_index;
  u64 index_count = 1;
  if(!present_shares_graphics_queue){
    index_count++;
  }
  if(!transfer_shares_graphics_queue){
    index_count++;
  }
  u64 indicies[index_count];
  u64 index = 0;
  indicies[index] = context->device.graphics_queue_family_index;
  if(!present_shares_graphics_queue){
    indicies[index++] = context->device.present_queue_family_index;
  }
  if(!transfer_shares_graphics_queue){
    indicies[index++] = context->device.transfer_queue_family_index;
  }
  VkDeviceQueueCreateInfo queue_create_infos[index_count];
  for(u64 i = 0; i < index_count; i++){
     queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
     queue_create_infos[i].queueFamilyIndex = indicies[i];
     queue_create_infos[i].queueCount = 1;
    //  if(indicies[i] == context->device.graphics_queue_family_index){
    //   queue_create_infos[i].queueCount = 2;
    //  }
    queue_create_infos[i].flags = 0;
    queue_create_infos[i].pNext = nullptr;
    float queue_priority = 1.0f;
    queue_create_infos[i].pQueuePriorities = &queue_priority;
  }
  //Request device features
  VkPhysicalDeviceFeatures device_features = {};
  device_features.samplerAnisotropy = VK_TRUE; // requesting anisotropy
  VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  device_create_info.queueCreateInfoCount = index_count;
  device_create_info.pQueueCreateInfos = queue_create_infos;
  device_create_info.pEnabledFeatures = &device_features;
  device_create_info.queueCreateInfoCount = index_count;
  device_create_info.enabledExtensionCount = 1;
  const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
  device_create_info.ppEnabledExtensionNames = &extension_names;
  device_create_info.enabledLayerCount = 0;
  device_create_info.flags= 0;
  device_create_info.pNext = nullptr;
 VK_CHECK(vkCreateDevice(context->device.physical_device,&device_create_info,context->allocator, &context->device.logical_device));
 DINFO("logical device created");

 vkGetDeviceQueue(context->device.logical_device,
                  context->device.graphics_queue_family_index,
                    0, &context->device.graphics);
  vkGetDeviceQueue(context->device.logical_device,
                  context->device.present_queue_family_index,
                    0, &context->device.present);
  vkGetDeviceQueue(context->device.logical_device,
                  context->device.transfer_queue_family_index,
                    0, &context->device.transfer); 
  DINFO("Queue obtained"); 
  context->device.supports_device_local_host_visible = true;
  //create command pool
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = (uint32_t)context->device.graphics_queue_family_index;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;// this ensures that we can reset our command buffers at any point by calling vkResetCmdBuffer or VkBeginBuffer that implicity resets the command buffer
    command_pool_create_info.pNext = nullptr;
   VK_CHECK(vkCreateCommandPool(context->device.logical_device, &command_pool_create_info,context->allocator,&context->device.graphics_command_pool));
   DINFO("Command pool successfully created");                                              
  return true;
}

void vulkan_device_destroy(vulkan_context*context){
  // unset queues
  context->device.graphics = 0;
  context->device.present = 0;
  context->device.transfer = 0;
  DINFO("Destroying command pool");
  if(context->device.graphics_command_pool){
    vkDestroyCommandPool(context->device.logical_device,context->device.graphics_command_pool, context->allocator);
    context->device.graphics_command_pool = 0;
  }
  //Destroying logical device
  DINFO("Destroying  logical device");
  if(context->device.logical_device){
    vkDestroyDevice(context->device.logical_device, context->allocator);
    context->device.logical_device = 0;
  }

  //Destroying physical device 
   DINFO("Releasing physical device resources");
   context->device.physical_device = nullptr;

   if(context->device.swapchain_support.surface_format){
    Dfree_memory(
       context->device.swapchain_support.surface_format, 
       MEMORY_TAG_RENDERER,
       sizeof(VkSurfaceFormatKHR)*context->device.swapchain_support.format_count
    );
    context->device.swapchain_support.surface_format = nullptr;
    context->device.swapchain_support.format_count = 0;
   }
   if(context->device.swapchain_support.present_mode){
    Dfree_memory(
       context->device.swapchain_support.present_mode, 
       MEMORY_TAG_RENDERER,
       sizeof(VkSurfaceFormatKHR)*context->device.swapchain_support.present_mode_count
    );
    context->device.swapchain_support.present_mode = nullptr;
    context->device.swapchain_support.present_mode_count = 0;
   }

   Dzero_memory(
    &context->device.swapchain_support.capabilities,
    sizeof(context->device.swapchain_support.capabilities));

   context->device.graphics_queue_family_index = -1;
   context->device.present_queue_family_index = -1;
   context->device.transfer_queue_family_index = -1;

}

void vulkan_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info*out_support_info
){
  // surface  capabilities 
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, 
                                                     &out_support_info->capabilities));
  // surface formats                                                   
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,surface,&out_support_info->format_count,
                                                nullptr));
  if (out_support_info->format_count !=0){
        if(!out_support_info->surface_format){
          out_support_info->surface_format =(VkSurfaceFormatKHR*) Dallocate_memory(sizeof(VkSurfaceFormatKHR)*out_support_info->format_count, MEMORY_TAG_RENDERER);
        }
      VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
          physical_device,
          surface,
          &out_support_info->format_count,
          out_support_info->surface_format
      ));
  }
  //present modes 
VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,surface,&out_support_info->present_mode_count, nullptr));
  if (out_support_info->present_mode_count !=0){
      if(!out_support_info->present_mode){
        out_support_info->present_mode =(VkPresentModeKHR*) Dallocate_memory(sizeof(VkPresentModeKHR)*out_support_info->present_mode_count, MEMORY_TAG_RENDERER);
      }
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
                  physical_device,
                  surface,
                  &out_support_info->present_mode_count, 
                  out_support_info->present_mode));
  }

};

bool select_physical_device(vulkan_context*context){
    uint32_t physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, nullptr));
    if(physical_device_count == 0){
      DFATAL("No physical device which support vulakn were found");
      return false;
    }
    VkPhysicalDevice physical_devices[physical_device_count]; // arrays of deviceses;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance,&physical_device_count, physical_devices));
    for(u32 i =0; i < physical_device_count; i++){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        requirements.compute = true;
        requirements.sampler_anisotropy = true;
        requirements.integrated_gpu = true;
        requirements.discrete_gpu = true;
        requirements.device_extension_names = darray_create(const char*);
        darray_push(requirements.device_extension_names,&VK_KHR_SWAPCHAIN_EXTENSION_NAME,const char*);
        vulkan_physical_device_queue_family_info  queque_info={};
       bool  result = physical_device_meets_requirements(
        physical_devices[i],
        &properties,
        &features,
        &requirements,
        &queque_info,
        &context->device.swapchain_support,
        context->surface 
       );
         
            if(result){

              DINFO("selected device: '%s' .", properties.deviceName);
                    switch(properties.deviceType){
                      default:
                      case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                          DINFO("GPU type is UNKNOWN"); 
                          break;    
                      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                          DINFO("GPU type is a integrated GPU");
                          break;
                      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                          DINFO("GPU type is a discrete GPU"); 
                          break;
                      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                          DINFO("GPU type is a virtual GPU"); 
                          break; 
                      case VK_PHYSICAL_DEVICE_TYPE_CPU:
                          DINFO("GPU type is a CPU"); 
                          break;               
                    }
                    DINFO(
                         "GPU driverversion:%d.%d.%d",
                          VK_VERSION_MAJOR(properties.driverVersion),
                          VK_API_VERSION_MINOR(properties.driverVersion),
                          VK_VERSION_PATCH(properties.driverVersion)); 
                     // vulkan API version
                         DINFO(
                         "vulkan API driverversion:%d.%d.%d",
                          VK_VERSION_MAJOR(properties.apiVersion),
                          VK_API_VERSION_MINOR(properties.apiVersion),
                          VK_VERSION_PATCH(properties.apiVersion));   
                    //memory information
                    for(u32 j= 0; j< memory.memoryHeapCount; j++){
                          float memory_size_gib = 
                          (((float)memory.memoryHeaps[j].size)/1024.0f/1024.0f/1024.0f);
                          if(memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT){
                            DINFO("local GPU memory: %.2f GiB", memory_size_gib);
                          }
                          else{
                            DINFO("shared system memory: %.2f GiB", memory_size_gib);
                          }
                    }   
                     context->device.physical_device = physical_devices[i];
                     context->device.graphics_queue_family_index= 
                     queque_info.graphics_queue_family_index; 
                     context->device.compute_queue_family_index= 
                     queque_info.compute_queue_family_index; 
                     context->device.transfer_queue_family_index= 
                     queque_info.transfer_queue_family_index;  
                     context->device.present_queue_family_index= 
                     queque_info.present_queue_family_index;   
                    

                    context->device.properties= properties;
                    context->device.features = features;
                    context->device.memory = memory;
                    break;
            } 
    }
    if(!context->device.physical_device){
      DERROR("No physical device were found which met the application's requirements");
      return false;
    }
   DINFO("physical device selected");
   return true;  
}

bool physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkPhysicalDeviceProperties* properties,
    VkPhysicalDeviceFeatures* features,
    vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info*out_queue_family_info,
    vulkan_swapchain_support_info*out_swapchain_support,
    VkSurfaceKHR surface){
    out_queue_family_info->compute_queue_family_index= -1;
    out_queue_family_info->graphics_queue_family_index =-1;
    out_queue_family_info->present_queue_family_index = -1;
    out_queue_family_info->transfer_queue_family_index = -1;
//gpu 
if(requirements->discrete_gpu){
     if(properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && properties->deviceType !=VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
        DINFO("Device is not a discrete gpu or an integreted gpu and one is required skipping");
        return false;
     }
     
   }
   uint32_t queue_family_count = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,nullptr);
   VkQueueFamilyProperties queue_families[queue_family_count]; // array of the struct
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,queue_families);

   DINFO("Graphics | present | compute | Transfer | Name");
   u8 min_transfer_score = 255;
for(u32 i =0; i < queue_family_count; i++){
    u32 current_transfer_score = 0;
    //graphics queue
    if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
        out_queue_family_info->graphics_queue_family_index = i;
        // DDEBUG("graphics Queue family queue count: %d", queue_families[i].queueCount);
        current_transfer_score++;
    }
      //transfer queue
    if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT){
        out_queue_family_info->transfer_queue_family_index = i;
        current_transfer_score++;
    //     DDEBUG("transfer Queue family queue count: %d", queue_families[i].queueCount);
     }

       //compute queue
    if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
        out_queue_family_info->compute_queue_family_index = i;
        current_transfer_score++;
    }
    //present queue
    VkBool32 support_present = VK_FALSE;
     vkGetPhysicalDeviceSurfaceSupportKHR(device,i,surface, &support_present);
    if(support_present){
        out_queue_family_info->present_queue_family_index = i;
      //  DDEBUG("present Queue family queue count: %d", queue_families[i].queueCount);
    }
 }
 //print out some info about the device 
 DINFO("    %d |        %d |         %d |       %d |  %s ", out_queue_family_info->graphics_queue_family_index != -1,  out_queue_family_info->present_queue_family_index != -1, out_queue_family_info->compute_queue_family_index != -1, out_queue_family_info->transfer_queue_family_index != -1,properties->deviceName);
 if(
     (!requirements->graphics || (requirements->graphics && out_queue_family_info->graphics_queue_family_index != -1)) && 
     (!requirements->present  || (requirements->present && out_queue_family_info->present_queue_family_index != -1))&&
     (!requirements->compute || (requirements->compute && out_queue_family_info->compute_queue_family_index != -1))&&
     (!requirements->transfer || (requirements->transfer && out_queue_family_info->transfer_queue_family_index != -1))
   ){
              DINFO("Device meets queue requirements");
              DTRACE("Graphics Family Index: %i", out_queue_family_info->graphics_queue_family_index);
              DTRACE("Present Family Index: %i", out_queue_family_info->present_queue_family_index);
              DTRACE("Transfer Family Index: %i", out_queue_family_info->transfer_queue_family_index);
              DTRACE("Compute Family Index: %i", out_queue_family_info->compute_queue_family_index);  


              vulkan_device_query_swapchain_support(
                device,
                surface,
                out_swapchain_support
              );  
              if(out_swapchain_support->format_count < 1 || out_swapchain_support->present_mode_count < 1){
                if(out_swapchain_support->surface_format){
                  Dfree_memory(out_swapchain_support->surface_format, MEMORY_TAG_RENDERER, sizeof(VkSurfaceFormatKHR)*out_swapchain_support->format_count);
                }
                if(out_swapchain_support->present_mode){
                  Dfree_memory(out_swapchain_support->present_mode, MEMORY_TAG_RENDERER, sizeof(VkPresentModeKHR)*out_swapchain_support->present_mode_count);
                }
                DINFO("Required swapchain support not present, skipping device");
                return false;
              }
           //Device extensions 
            if(requirements->device_extension_names){
                    u32 available_extension_count = 0;
                    VkExtensionProperties*available_extension = 0;
                    VK_CHECK(vkEnumerateDeviceExtensionProperties(
                      device,
                      nullptr,
                      &available_extension_count,
                      nullptr
                    ));
                    if(available_extension_count != 0){
                      available_extension = (VkExtensionProperties*)Dallocate_memory(sizeof(VkExtensionProperties)*available_extension_count, MEMORY_TAG_RENDERER);

                          VK_CHECK(vkEnumerateDeviceExtensionProperties(
                          device,
                          nullptr,
                          &available_extension_count,
                          available_extension
                        ));
                    }

                u64 required_extension_count = darray_get_length(requirements->device_extension_names);
                for(u64 i = 0; i < required_extension_count; i++){
                    bool found = false;
                        for(u64 j =0;  j < available_extension_count; j++){
                              if(string_equal(requirements->device_extension_names[i], available_extension[j].extensionName)){
                                found = true;
                                break;
                              }
                          }
                      if(!found){
                           DINFO("Required extension not found: %s, skipping device", requirements->device_extension_names[i]);
                           Dfree_memory(available_extension, MEMORY_TAG_RENDERER, sizeof(VkExtensionProperties)*available_extension_count);
                           return false;
                          }
                  }
                  Dfree_memory(available_extension, MEMORY_TAG_RENDERER, sizeof(VkExtensionProperties)*available_extension_count);
              
         }
         //sampler anisotropy 
       if(requirements->sampler_anisotropy && !features->samplerAnisotropy){
          DINFO("Device does not support samplerAnisotropy, skipping");
          return false;
         }
     return true;     
   };
   return false; 
}
bool vulkan_device_detect_depth_format(vulkan_device*device){
  //format candidates
  const u64 candidate_count = 3;
  VkFormat candidates[3] ={
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT
  };
  u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
  for(u64 i = 0; i < candidate_count; i++){
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(device->physical_device,candidates[i],&properties);
    if((properties.optimalTilingFeatures & flags)== flags){
      device->depth_format = candidates[i];
      return true;
    }
    else if((properties.linearTilingFeatures & flags)== flags){
        device->depth_format = candidates[i];
        return true;
    }
  }
  return false;
}