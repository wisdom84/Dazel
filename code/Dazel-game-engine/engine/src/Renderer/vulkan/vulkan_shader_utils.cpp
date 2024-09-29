#include "vulkan_shader_utils.h"
#include "containers/Darray.h"
#include "core/Dstrings.h"
#include "core/logger.h"
#include "systems/resource_system.h"


bool create_shader_stage(vulkan_context*context,const char* shader_ext,
                          VkShaderStageFlagBits shader_stage_flag, u32 stage_index,
                          vulkan_shader_stage*shader_stage 
){
resource binary_data;
char full_path[512];
string_format(full_path, "shader_bin/%s.spv", shader_ext);
if(!resource_system_load(full_path,RESOURCE_TYPE_BINARY, &binary_data)){
  DERROR("unable to read binary file");
  return false;
}
u8*data = (u8*)binary_data.data;
u64 size = binary_data.size_of_data;

shader_stage[stage_index].create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
shader_stage[stage_index].create_info.flags = 0;
shader_stage[stage_index].create_info.pNext = nullptr;
shader_stage[stage_index].create_info.pCode = reinterpret_cast<const uint32_t*>(data);
shader_stage[stage_index].create_info.codeSize =(size_t)size;
// create shader module
VK_CHECK(vkCreateShaderModule(context->device.logical_device, &shader_stage[stage_index].create_info, context->allocator,&shader_stage[stage_index].handle));
//  fill in the pipeline shader stage create info 
shader_stage[stage_index].pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
shader_stage[stage_index].pipeline_shader_stage_create_info.flags = 0;
shader_stage[stage_index].pipeline_shader_stage_create_info.module = shader_stage[stage_index].handle;
shader_stage[stage_index].pipeline_shader_stage_create_info.pSpecializationInfo = nullptr;
shader_stage[stage_index].pipeline_shader_stage_create_info.pNext = nullptr;
shader_stage[stage_index].pipeline_shader_stage_create_info.stage = shader_stage_flag;
shader_stage[stage_index].pipeline_shader_stage_create_info.pName = "main";
resource_system_unload(&binary_data);
return true;
};