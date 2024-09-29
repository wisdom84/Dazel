#include "vulkan_object_ui_shaders.h"
#include "Renderer/vulkan/vulkan_buffer.h"
#include "Renderer/vulkan/vulkan_shader_utils.h"
#include "Renderer/vulkan/vulkan_pipeline.h"
#include "core/logger.h"
#include "Math/dlm.h"
#include "core/Dstrings.h"
#include "systems/texture_system.h"
using namespace dlm;

bool vulkan_shader_object_UI_create(vulkan_context*context,vulkan_shader_UI_object*out_shader){
     char shader_stage_type [2][7] = {"UIvert", "UIfrag"};
     VkShaderStageFlagBits shader_stage_types_flags[2] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
     for(auto i = 0; i < 2; i++){
       if(!create_shader_stage(context,shader_stage_type[i],shader_stage_types_flags[i],i,out_shader->shader_stages)){
          DERROR("failed to create the %s shader stage ", shader_stage_type[i]);
          return false;
        }
     }
     //global descriptors
  VkDescriptorSetLayoutBinding global_ubo_layout_binding;
  global_ubo_layout_binding.binding = 0;
  global_ubo_layout_binding.descriptorCount = 1; // the number of descriptor in a descriptor set
  global_ubo_layout_binding.descriptorType =VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  global_ubo_layout_binding.pImmutableSamplers = nullptr;
  global_ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  
  VkDescriptorSetLayoutCreateInfo global_ubo_layout_create_info;
  global_ubo_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  global_ubo_layout_create_info.pNext = nullptr;
  global_ubo_layout_create_info.bindingCount = 1;
  global_ubo_layout_create_info.pBindings =&global_ubo_layout_binding;
  global_ubo_layout_create_info.flags=0;
  VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device,&global_ubo_layout_create_info,context->allocator,&out_shader->global_descriptor_set_layout));

  VkDescriptorPoolSize global_ubo_pool_size;
  global_ubo_pool_size.descriptorCount = (uint32_t)context->swapchain.image_count;
  global_ubo_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  VkDescriptorPoolCreateInfo global_ubo_pool_create_info;
  global_ubo_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  global_ubo_pool_create_info.pPoolSizes=&global_ubo_pool_size;
  global_ubo_pool_create_info.poolSizeCount = 1;
  global_ubo_pool_create_info.pNext = nullptr;
  global_ubo_pool_create_info.maxSets =(uint32_t)context->swapchain.image_count;
  global_ubo_pool_create_info.flags = 0;
  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device,& global_ubo_pool_create_info,context->allocator,&out_shader->global_descriptor_pool));

VkDescriptorSetLayout descriptor_layout[3]={out_shader->global_descriptor_set_layout,
                                 out_shader->global_descriptor_set_layout,
                                 out_shader->global_descriptor_set_layout};

VkDescriptorSetAllocateInfo descriptor_set_allocate_info;
descriptor_set_allocate_info.sType =VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
descriptor_set_allocate_info.pNext = nullptr;
descriptor_set_allocate_info.descriptorSetCount = (uint32_t)(context->swapchain.image_count);
descriptor_set_allocate_info.descriptorPool = out_shader->global_descriptor_pool;
descriptor_set_allocate_info.pSetLayouts =descriptor_layout;
VK_CHECK(vkAllocateDescriptorSets(context->device.logical_device,&descriptor_set_allocate_info,out_shader->global_descriptor_set)); 
// local Descriptors
out_shader->sampler_uses[0] = TEXTURE_USE_MAP_DIFFUSE;
  VkDescriptorType descriptor_type[VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT]={VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

  VkDescriptorPoolSize object_ubo_pool_size[VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT];

  object_ubo_pool_size[0].descriptorCount = VULKAN_OBJECT_UI_MAX_COUNT;
  object_ubo_pool_size[0].type = descriptor_type[0];

  object_ubo_pool_size[1].descriptorCount = VULKAN_OBJECT_SHADER_UI_SAMPLER_COUNT*VULKAN_OBJECT_UI_MAX_COUNT;
  object_ubo_pool_size[1].type = descriptor_type[1];

  VkDescriptorPoolCreateInfo object_ubo_pool_create_info;
  object_ubo_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  object_ubo_pool_create_info.pPoolSizes=object_ubo_pool_size;
  object_ubo_pool_create_info.poolSizeCount = 2;
  object_ubo_pool_create_info.pNext = nullptr;
  object_ubo_pool_create_info.maxSets =VULKAN_OBJECT_UI_MAX_COUNT;
  object_ubo_pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  VK_CHECK(vkCreateDescriptorPool(context->device.logical_device,& object_ubo_pool_create_info,context->allocator,&out_shader->object_descriptor_pool));


VkDescriptorSetLayoutBinding bindings[VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT];
for(u32 i=0; i < VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT; i++){
  bindings[i].binding =i;
  bindings[i].descriptorCount = 1; // the number of descriptor in a descriptor set
  bindings[i].descriptorType =descriptor_type[i];
  bindings[i].pImmutableSamplers = nullptr;
  bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
}
  VkDescriptorSetLayoutCreateInfo object_ubo_layout_create_info;
  object_ubo_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  object_ubo_layout_create_info.pNext = nullptr;
  object_ubo_layout_create_info.bindingCount = VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT;
  object_ubo_layout_create_info.pBindings =bindings;
  object_ubo_layout_create_info.flags=0;
  VK_CHECK(vkCreateDescriptorSetLayout(context->device.logical_device,&object_ubo_layout_create_info,context->allocator,&out_shader->object_descriptor_set_layout));
   VkDescriptorSetLayout total_set_layout[2]={out_shader->global_descriptor_set_layout, out_shader->object_descriptor_set_layout};
     // graphics pipeline 
     VkViewport viewport{};
     viewport.x = 0.0; // the region in the frame buffer we want to start excuting our draw operation. in this case we are starting from the origin
     viewport.y = (float)context->swapchain.extent.height;
     viewport.width = (float)context->swapchain.extent.width;
     viewport.height = -(float)context->swapchain.extent.height;
     viewport.minDepth = 0.0f; // mindepth and maxdepth of the images are in the bounds (0-1) if we want to use depth we need a GPU feature
     viewport.maxDepth = 1.0f;

     VkRect2D scissor{};
     scissor.extent.width = (float)context->swapchain.extent.width;
     scissor.extent.height = (float)context->swapchain.extent.height;
     scissor.offset.x = 0;
     scissor.offset.y = 0;
     //Attributes 
     u32 offset = 0;
     const int attribute_count = 2;
     VkVertexInputAttributeDescription attribute_description[attribute_count];
     //position
     VkFormat formats[attribute_count]={
      VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32_SFLOAT
     };
     u64 sizes[attribute_count] = {
      sizeof(vec2),
      sizeof(vec2)
     };
     for(u32 i= 0; i < attribute_count; i++){
      attribute_description[i].location = i;
      attribute_description[i].format = formats[i];
      attribute_description[i].offset=(uint32_t)offset;
      attribute_description[i].binding = 0;
      offset += sizes[i];
     }
     VkPipelineShaderStageCreateInfo shader_stage_create_info[2]={
         out_shader->shader_stages[1].pipeline_shader_stage_create_info,
         out_shader->shader_stages[0].pipeline_shader_stage_create_info  
     };
     u64 total_descriptor_set_layout_count = 2;
    
    vulkan_graphics_pipeline_create(
      context,
      &context->UI_renderpass,
      sizeof(Vertex_2d),
      attribute_count,
      attribute_description,
      total_set_layout,
      total_descriptor_set_layout_count,
      2,
      shader_stage_create_info,
      viewport,
      scissor,
      false,
      false,
      &out_shader->pipeline
    );
    // create out UBO
    if(!vulkan_buffer_create(
      context,
      sizeof(global_UI_uniform_object),
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      true,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      &out_shader->global_uniform_buffer
    )){
      DERROR("failed to create uniform buffer");
      return false;
    }
    //create object uniform buffer
     if(!vulkan_buffer_create(
      context,
      sizeof(uniform_UI_object)*VULKAN_OBJECT_UI_MAX_COUNT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      true,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      &out_shader->object_uniform_buffer
    )){
      DERROR("failed to create uniform buffer");
      return false;
    }
   return true; 
}

void vulkan_shader_object_UI_destroy(vulkan_context*context, vulkan_shader_UI_object*out_shader){
  vulkan_buffer_destroy(context,&out_shader->global_uniform_buffer);
  vulkan_buffer_destroy(context,&out_shader->object_uniform_buffer);

  vulkan_graphics_pipeline_destroy(context,&out_shader->pipeline);

  vkDestroyDescriptorPool(context->device.logical_device,out_shader->global_descriptor_pool,context->allocator);

  vkDestroyDescriptorSetLayout(context->device.logical_device,out_shader->global_descriptor_set_layout,context->allocator);

  vkDestroyDescriptorPool(context->device.logical_device,out_shader->object_descriptor_pool,context->allocator);

  vkDestroyDescriptorSetLayout(context->device.logical_device,out_shader->object_descriptor_set_layout,context->allocator);
  for(auto i = 0; i < 2; i++){
    vkDestroyShaderModule(context->device.logical_device,out_shader->shader_stages[i].handle, context->allocator);
    out_shader->shader_stages[i].handle = nullptr;
  }
 
}

void vulkan_object_shader_UI_use(vulkan_context*context,vulkan_shader_UI_object*shader){
  u32 image_index = context->image_index;
  vulkan_bind_pipeline(&context->graphics_command_buffers[image_index], VK_PIPELINE_BIND_POINT_GRAPHICS,&shader->pipeline);
  //binding descriptor set for use
  vkCmdBindDescriptorSets(context->graphics_command_buffers[image_index].handle,
                             VK_PIPELINE_BIND_POINT_GRAPHICS,
                             context->built_in_UI_object_shader.pipeline.layout_handle,
                             0,1,
                             &context->built_in_UI_object_shader.global_descriptor_set[image_index],
                             0,nullptr);
}


void vulkan_shader_object_update_global_UI_state(vulkan_context*context, vulkan_shader_UI_object*shader,float delta_time){
  u32 range = sizeof(global_UI_uniform_object);
  u64 offset = 0;
  //copy data to the uniform buffer


  VkDescriptorBufferInfo  global_ubo_descriptor_buffer;
  global_ubo_descriptor_buffer.buffer = shader->global_uniform_buffer.handle;
  global_ubo_descriptor_buffer.offset =offset;
  global_ubo_descriptor_buffer.range = range; // the size of each piece of data in the buffer


  VkWriteDescriptorSet global_ubo_write_descriptor;
  global_ubo_write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  global_ubo_write_descriptor.pNext = nullptr;
  global_ubo_write_descriptor.descriptorCount = 1;
  global_ubo_write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  global_ubo_write_descriptor.dstArrayElement = 0;
  global_ubo_write_descriptor.dstBinding = 0;
  global_ubo_write_descriptor.dstSet = shader->global_descriptor_set[context->image_index];
  global_ubo_write_descriptor.pBufferInfo = &global_ubo_descriptor_buffer;
  global_ubo_write_descriptor.pImageInfo = nullptr;
  global_ubo_write_descriptor.pTexelBufferView = nullptr;

  vkUpdateDescriptorSets(context->device.logical_device,1,&global_ubo_write_descriptor,0,nullptr);
  vulkan_buffer_load_data(context,&shader->global_uniform_buffer,offset,range,0,&shader->global_ubo);

}
void vulkan_object_shader_UI_set_model(vulkan_context*context,  vulkan_shader_UI_object*shader, mat4 model){
     if(context &&  shader){
          u32 image_index = context->image_index;
          vulkan_command_buffer*command_buffer =&context->graphics_command_buffers[image_index];
          VkShaderStageFlags flag = VK_SHADER_STAGE_VERTEX_BIT;
        vkCmdPushConstants(command_buffer->handle, shader->pipeline.layout_handle,flag,0, sizeof(mat4), &model);
     }
   };
void vulkan_object_shader_UI_apply_material(vulkan_context*context,  vulkan_shader_UI_object*shader, materials*material){
      u32 image_index = context->image_index;
      vulkan_command_buffer*command_buffer =&context->graphics_command_buffers[image_index];
     if(context && shader){
              // obtain material data
              vulkan_objcet_shader_UI_object_state* object_state = &shader->object_states[material->internal_id];
              VkDescriptorSet object_descriptor_set = object_state->descriptor_sets[image_index];

              VkWriteDescriptorSet descriptor_writes[VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT];
              u32 descriptor_count = 0;
              u32 descriptor_index = 0;

              // Descriptor 0 - uniform_buffer
              u32 range = sizeof(uniform_UI_object);
              u64 offset = sizeof(uniform_UI_object) * material->internal_id;
              uniform_UI_object object;
            object.diffuse_color = material->diffuse_color;
          
          
              u32*object_state_generation = &object_state->descriptor_state[descriptor_index].generation[image_index];
            if(*object_state_generation== INVALID_ID){
                VkDescriptorBufferInfo buffer_info;
                buffer_info.buffer = shader->object_uniform_buffer.handle;
                buffer_info.offset = offset;
                buffer_info.range = range;

                VkWriteDescriptorSet object_ubo_write_descriptor;
                object_ubo_write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                object_ubo_write_descriptor.pNext = nullptr;
                object_ubo_write_descriptor.descriptorCount = 1;
                object_ubo_write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                object_ubo_write_descriptor.dstArrayElement = 0;
                object_ubo_write_descriptor.dstBinding = descriptor_index;
                object_ubo_write_descriptor.dstSet = object_descriptor_set;
                object_ubo_write_descriptor.pBufferInfo = &buffer_info;
                object_ubo_write_descriptor.pImageInfo = nullptr;
                object_ubo_write_descriptor.pTexelBufferView = nullptr;

                descriptor_writes[descriptor_count]= object_ubo_write_descriptor;
                descriptor_count++;
              *object_state_generation= material->generations;

            }
            descriptor_index++;

            // descriptors with samplers
            const u32 sampler_count = 1;
            VkDescriptorImageInfo image_info[1];
            for(u32 sampler_index=0; sampler_index < sampler_count; sampler_index++){
              texture_use  use = shader->sampler_uses[sampler_index];
              Texture*texture = 0;
              switch (use)
              {
              case TEXTURE_USE_MAP_DIFFUSE:
                    texture = material->diffuse_map.texture;
                    break;
              
              default:
                DFATAL("Unable to bind sampler to unknown use");
                break;
              }
              u32*descriptor_generation = &object_state->descriptor_state[descriptor_index].generation[image_index];
              u32*descriptor_id = &object_state->descriptor_state[descriptor_index].id[image_index];
              if(texture->generation == INVALID_ID){
                texture = texture_system_get_defualt_texture();
                *descriptor_generation = INVALID_ID;
              }
              if(texture && (*descriptor_id != texture->id || *descriptor_generation != texture->generation || *descriptor_generation == INVALID_ID)){
                vulkan_texture_data*internal_data = (vulkan_texture_data*)texture->internal_data;

                // assign the sampler and the view
                image_info[sampler_index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_info[sampler_index].imageView = internal_data->image.view;
                image_info[sampler_index].sampler = internal_data->sampler;
              
                VkWriteDescriptorSet object_ubo_write_descriptor;
                object_ubo_write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                object_ubo_write_descriptor.pNext = nullptr;
                object_ubo_write_descriptor.descriptorCount = 1;
                object_ubo_write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                object_ubo_write_descriptor.dstArrayElement = 0;
                object_ubo_write_descriptor.dstBinding = descriptor_index;
                object_ubo_write_descriptor.dstSet = object_descriptor_set;
                object_ubo_write_descriptor.pBufferInfo = nullptr;
                object_ubo_write_descriptor.pImageInfo = &image_info[sampler_index];
                object_ubo_write_descriptor.pTexelBufferView = nullptr;

                
                descriptor_writes[descriptor_count]= object_ubo_write_descriptor;
                descriptor_count++;
                if(texture->generation != INVALID_ID){
                  *descriptor_generation = texture->generation;
                  *descriptor_id = texture->id;
                }
              descriptor_index++;
              }
            }
            if(descriptor_count > 0){
              vkUpdateDescriptorSets(context->device.logical_device,descriptor_count,descriptor_writes,0,nullptr);
              vulkan_buffer_load_data(context,&shader->object_uniform_buffer,offset, range,0, &object);
            }
          vkCmdBindDescriptorSets(context->graphics_command_buffers[image_index].handle,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      shader->pipeline.layout_handle,
                                      1,1,
                                      &object_descriptor_set,
                                      0,nullptr);
  } 

}

bool vulkan_shader_object_UI_aquire_resources(vulkan_context*context,struct vulkan_shader_UI_object*shader, materials*material){
  material->internal_id = shader->object_uniform_buffer_index;
  shader->object_uniform_buffer_index++;
  vulkan_objcet_shader_UI_object_state*object_state = &shader->object_states[material->internal_id];
  for(u32 i = 0; i < VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT; i++){
    for(u32 j=0; j < 3; j++){
      object_state->descriptor_state[i].generation[j]= INVALID_ID;
      object_state->descriptor_state[i].id[j]= INVALID_ID;
    }
  }
  //Allocate descriptor sets 
  VkDescriptorSetLayout layouts[3]={shader->object_descriptor_set_layout,
                                    shader->object_descriptor_set_layout,
                                    shader->object_descriptor_set_layout};
VkDescriptorSetAllocateInfo descriptor_set_allocate_info;
descriptor_set_allocate_info.sType =VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
descriptor_set_allocate_info.pNext = nullptr;
descriptor_set_allocate_info.descriptorSetCount =3;
descriptor_set_allocate_info.descriptorPool = shader->object_descriptor_pool;
descriptor_set_allocate_info.pSetLayouts =layouts;
VkResult result = vkAllocateDescriptorSets(context->device.logical_device,&descriptor_set_allocate_info,shader->object_states[material->internal_id].descriptor_sets);
if(result != VK_SUCCESS){
  DERROR("failed to allocate descriptor sets in shader");
  return false;
}                                  
                                   
  return true;
}

void vulkan_shader_object_UI_release_resources(vulkan_context*context, struct vulkan_shader_UI_object*shader,materials*material){
  vulkan_objcet_shader_UI_object_state*object_state = &shader->object_states[material->internal_id];
  const u32 descriptor_set_count = 3;
  vkDeviceWaitIdle(context->device.logical_device);
  VkResult result = vkFreeDescriptorSets(context->device.logical_device, shader->object_descriptor_pool, descriptor_set_count,object_state[material->internal_id].descriptor_sets);
   if(result != VK_SUCCESS){
     DERROR("failed to free descriptor sets from the descriptor pool");
     return;
   }
 for(u32 i = 0; i < VULKAN_OBJECT_SHADER_UI_DESCRIPTOR_COUNT; i++){
    for(u32 j=0; j < 3; j++){
      object_state->descriptor_state[i].generation[j]=INVALID_ID;
      object_state->descriptor_state[i].id[j]= INVALID_ID;
    }
  }
  material->internal_id = INVALID_ID;
};