#include "vulkan_pipeline.h"
#include "core/logger.h"
#include "core/Dmemory.h"
#include "core/Dstrings.h"
#include "vulkan_utils.h"
#include "Math/dlm.h"
using namespace dlm;

bool vulkan_graphics_pipeline_create(
    vulkan_context*context,
    vulkan_renderpass*renderpass,
    u32 stride,
    u32 attribute_count,
    VkVertexInputAttributeDescription*attribute_description,
    VkDescriptorSetLayout*descriptor_set_layout,
    u64 descriptor_set_layout_count,
    u32 stage_count,
    VkPipelineShaderStageCreateInfo*shader_stages,
    VkViewport viewport,
     VkRect2D scissor,
    bool is_wireframe,
    bool depth_test_enabled,
    vulkan_pipeline*out_pipeline
){
    //Viewport state
VkPipelineViewportStateCreateInfo view_port_state;
view_port_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
view_port_state.pNext = nullptr;
view_port_state.flags = 0;
view_port_state.scissorCount = 1;
view_port_state.viewportCount = 1;
view_port_state.pViewports = &viewport;
view_port_state.pScissors = &scissor;
  // Rasterization state  
VkPipelineRasterizationStateCreateInfo Rasterization_state_create_info;
Rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
Rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE; // only in wireframe builds
Rasterization_state_create_info.polygonMode =is_wireframe? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
Rasterization_state_create_info.lineWidth = 1.0f;
Rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
Rasterization_state_create_info.depthClampEnable = VK_FALSE;
Rasterization_state_create_info.depthBiasEnable = VK_FALSE;
Rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
Rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
Rasterization_state_create_info.depthBiasClamp = 0.0f;
Rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
Rasterization_state_create_info.pNext = nullptr;
Rasterization_state_create_info.flags = 0;
  // multisampling state
VkPipelineMultisampleStateCreateInfo multisampling_state_create_info;
multisampling_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
multisampling_state_create_info.pNext = nullptr;
multisampling_state_create_info.alphaToOneEnable =VK_FALSE;
multisampling_state_create_info.alphaToCoverageEnable = VK_FALSE;
multisampling_state_create_info.minSampleShading =0.0f;
multisampling_state_create_info.pSampleMask = nullptr;
multisampling_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
multisampling_state_create_info.sampleShadingEnable  = VK_FALSE;
multisampling_state_create_info.flags= 0;
 // Depth Stencil state create info of the pipeline
 VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info; 
 if(depth_test_enabled){
        depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_state_create_info.depthTestEnable =  VK_TRUE; // allows us to be able to do depth testing 
        depth_stencil_state_create_info.depthWriteEnable = VK_TRUE; // enable us to write to the ddepth buffer images 
        depth_stencil_state_create_info.flags = 0;
        depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS; // this lets us to say that  the mesh with less z value comes in front 
        depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_state_create_info.maxDepthBounds = 0.0f;
        depth_stencil_state_create_info.minDepthBounds = 0.0f;
        depth_stencil_state_create_info.pNext = nullptr;
        depth_stencil_state_create_info.stencilTestEnable = VK_FALSE; // for now we are not planning to use the stencil proprety of the depth buffer
        depth_stencil_state_create_info.front.failOp = VK_STENCIL_OP_ZERO;
        depth_stencil_state_create_info.front.passOp = VK_STENCIL_OP_ZERO;
        depth_stencil_state_create_info.front.depthFailOp = VK_STENCIL_OP_ZERO;
        depth_stencil_state_create_info.front.compareOp = VK_COMPARE_OP_NEVER;
        depth_stencil_state_create_info.front.compareMask = 0;
        depth_stencil_state_create_info.front.reference = 0;
        depth_stencil_state_create_info.front.writeMask = 0;
        depth_stencil_state_create_info.back.failOp = VK_STENCIL_OP_ZERO;
        depth_stencil_state_create_info.back.passOp = VK_STENCIL_OP_ZERO;
        depth_stencil_state_create_info.back.depthFailOp = VK_STENCIL_OP_ZERO;
        depth_stencil_state_create_info.back.compareOp = VK_COMPARE_OP_NEVER;
        depth_stencil_state_create_info.back.compareMask = 0;
        depth_stencil_state_create_info.back.reference = 0;
        depth_stencil_state_create_info.back.writeMask = 0;
 }



//Color blend attachment state
VkPipelineColorBlendAttachmentState blend_attachment ;
blend_attachment.blendEnable = VK_TRUE;
blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // keeping the alpha value of the source color
blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // setting the dst color alpha value to zero
blend_attachment.colorWriteMask= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT|
                                VK_COLOR_COMPONENT_A_BIT;
 // color blend state
VkPipelineColorBlendStateCreateInfo color_blend_state_create_info;
color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
color_blend_state_create_info.pAttachments = &blend_attachment;
color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
color_blend_state_create_info.attachmentCount = 1;
color_blend_state_create_info.flags = 0;
color_blend_state_create_info.pNext = nullptr;
color_blend_state_create_info.logicOpEnable = VK_FALSE;
color_blend_state_create_info.blendConstants[0] = 0.0f;
color_blend_state_create_info.blendConstants[1] = 0.0f;
color_blend_state_create_info.blendConstants[2] = 0.0f;
color_blend_state_create_info.blendConstants[3] = 0.0f;

 // set up the dynamic state for our view port and sccissor 
VkDynamicState dynamic_state[3] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,VK_DYNAMIC_STATE_LINE_WIDTH};
VkPipelineDynamicStateCreateInfo dynamic_state_create_info;
dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
dynamic_state_create_info.dynamicStateCount = 3;
dynamic_state_create_info.flags = 0;
dynamic_state_create_info.pDynamicStates=dynamic_state;
dynamic_state_create_info.pNext = nullptr;  

//Vertex input 
VkVertexInputBindingDescription binding_description;
binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
binding_description.stride= (uint32_t)stride;
binding_description.binding = 0;

VkPipelineVertexInputStateCreateInfo vertex_input_state;
vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vertex_input_state.pNext = nullptr;
vertex_input_state.pVertexAttributeDescriptions=attribute_description;
vertex_input_state.pVertexBindingDescriptions = &binding_description;
vertex_input_state.vertexAttributeDescriptionCount = attribute_count;
vertex_input_state.vertexBindingDescriptionCount = 1;
vertex_input_state.flags = 0;

//input assembly
VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info ;
input_assembly_create_info .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
input_assembly_create_info .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
input_assembly_create_info.primitiveRestartEnable = VK_FALSE;
input_assembly_create_info.pNext = nullptr;
input_assembly_create_info.flags= 0;

//PUSH constant values
VkPushConstantRange push_constants;
push_constants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
push_constants.size = sizeof(mat4)*2; // we are multipling by two to give us 128 bytes each mat4 is 64 bytes
push_constants.offset = 0;
  //pipeline layout   
VkPipelineLayoutCreateInfo pipeline_layout;
pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
pipeline_layout.pNext = nullptr;
pipeline_layout.setLayoutCount =(uint32_t)descriptor_set_layout_count;
pipeline_layout.pPushConstantRanges =&push_constants;
pipeline_layout.flags = 0;
pipeline_layout.pSetLayouts =descriptor_set_layout;
pipeline_layout.pushConstantRangeCount = 1; 
    
  VK_CHECK(vkCreatePipelineLayout(context->device.logical_device
                                    ,&pipeline_layout,
                                    context->allocator
                                    ,&out_pipeline->layout_handle));

VkGraphicsPipelineCreateInfo graphics_pipeline_create_info;
graphics_pipeline_create_info.sType= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
graphics_pipeline_create_info.layout = out_pipeline->layout_handle;
graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
graphics_pipeline_create_info.pDepthStencilState =depth_test_enabled? &depth_stencil_state_create_info: nullptr;
graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
graphics_pipeline_create_info.pVertexInputState = &vertex_input_state;
graphics_pipeline_create_info.pViewportState = &view_port_state;
graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
graphics_pipeline_create_info.pMultisampleState = &multisampling_state_create_info;
graphics_pipeline_create_info.pRasterizationState = &Rasterization_state_create_info;
graphics_pipeline_create_info.pStages = shader_stages;
graphics_pipeline_create_info.stageCount = (uint32_t)stage_count;
graphics_pipeline_create_info.pTessellationState = nullptr;
graphics_pipeline_create_info.renderPass=renderpass->handle;
graphics_pipeline_create_info.subpass =  0; // index of the subpass
graphics_pipeline_create_info.basePipelineIndex = 0; // index of the pipeline we want to use in this case we only have one pipeline
graphics_pipeline_create_info.basePipelineHandle = nullptr;
graphics_pipeline_create_info.pNext = nullptr;
graphics_pipeline_create_info.flags = 0;  
VkResult result = vkCreateGraphicsPipelines(context->device.logical_device,nullptr,1,&graphics_pipeline_create_info,context->allocator,&out_pipeline->handle);
if(vulkan_result_is_success(result)){
    DDEBUG("Graphics pipeline created successfully");
    return true;
} 
DERROR("vkCreateGraphicsPipelines failed with error %s.",vulkan_result_string(result,true));                                                                   
    return false;
}

void vulkan_graphics_pipeline_destroy( vulkan_context*context,vulkan_pipeline*pipeline){
    if(pipeline){
            if(pipeline->handle){
                vkDestroyPipeline(context->device.logical_device,pipeline->handle,context->allocator);
                pipeline->handle= 0;
            }
            if(pipeline->layout_handle){
                vkDestroyPipelineLayout(context->device.logical_device,pipeline->layout_handle,context->allocator);
                pipeline->layout_handle = 0;
            }
    }
};

void vulkan_bind_pipeline(vulkan_command_buffer*command_buffer, VkPipelineBindPoint bind_point,vulkan_pipeline*pipeline){
    vkCmdBindPipeline(command_buffer->handle, bind_point,pipeline->handle);
};