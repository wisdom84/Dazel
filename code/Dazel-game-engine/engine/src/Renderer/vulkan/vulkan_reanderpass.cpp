#include "vulkan_renderpass.h"
#include "core/Dmemory.h"
#include "core/logger.h"
void vulkan_renderpass_create(
     vulkan_context*context,
     vulkan_renderpass*out_renderpass,
     float x, float y, float w, float h,
     float r, float g, float b, float a,
     float depth,
     u64 stencil,
     u8 clear_flag,
     bool has_prev_pass,
     bool has_next_pass
){
   out_renderpass->x= x;  out_renderpass->y= y;
   out_renderpass->r= r;  out_renderpass->g= g;
   out_renderpass->b= b;  out_renderpass->a= a;
   out_renderpass->w = w; out_renderpass->h= h;
   out_renderpass->depth= depth; out_renderpass->stencil= stencil;
   out_renderpass->clear_flag = clear_flag; out_renderpass->has_prev_pass = has_prev_pass;
   out_renderpass->has_next_pass = has_next_pass;
    VkSubpassDescription subpass={};
    u32 attachment_description_count = 0;
    VkAttachmentDescription Attachments[2];
    bool do_clear_color = (out_renderpass->clear_flag & RENDERPASS_CLEAR_COLOR_BUFFER_FLAG) != 0;
    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format =context->swapchain.image_format.format;
    color_attachment.initialLayout = has_prev_pass? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.loadOp = do_clear_color?VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment.samples =VK_SAMPLE_COUNT_1_BIT;
    color_attachment.stencilLoadOp =  VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp =  VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.finalLayout = has_next_pass? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      //Depth attachment reference 
    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    Attachments[attachment_description_count]= color_attachment;
    attachment_description_count++;
    bool do_clear_depth = (out_renderpass->clear_flag & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) != 0;
   if(do_clear_depth){
  
      VkAttachmentDescription depth_attachment;
      depth_attachment.flags = 0;
      depth_attachment.format =context->device.depth_format;
      depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      depth_attachment.loadOp = do_clear_depth?VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
      depth_attachment.samples =VK_SAMPLE_COUNT_1_BIT;
      depth_attachment.stencilLoadOp =  VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      depth_attachment.stencilStoreOp =  VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      
      VkAttachmentReference depth_attachment_reference;
      depth_attachment_reference.attachment = 1;
      depth_attachment_reference.layout =  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      Attachments[attachment_description_count]= depth_attachment;
      attachment_description_count++;
      // if we have depth attachment on this renderpass
      subpass.pDepthStencilAttachment = &depth_attachment_reference;
   }else{
     Dzero_memory(&Attachments[attachment_description_count], sizeof(VkAttachmentDescription));
     subpass.pDepthStencilAttachment =nullptr;
   }
  
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags =0;
    subpass.inputAttachmentCount = 0;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pInputAttachments=nullptr;
    subpass.pPreserveAttachments = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pResolveAttachments = nullptr;
    //Render_pass dependency
    VkSubpassDependency dependency[2];
    dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency[0].dependencyFlags = 0;
    dependency[0].dstSubpass= 0;
    dependency[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependency[1].srcSubpass = 0;
    dependency[1].dependencyFlags = 0;
    dependency[1].dstSubpass= VK_SUBPASS_EXTERNAL;
    dependency[1].srcStageMask =VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT ;


    //Render pass create
    VkRenderPassCreateInfo render_pass_create_info={VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_create_info.flags = 0;
    render_pass_create_info.pNext = nullptr;
    render_pass_create_info.pSubpasses =  &subpass;
    render_pass_create_info.dependencyCount = 2;
    render_pass_create_info.pDependencies  = dependency;
    render_pass_create_info.attachmentCount = attachment_description_count;
    render_pass_create_info.pAttachments = Attachments;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pAttachments = Attachments;
   VK_CHECK(vkCreateRenderPass(context->device.logical_device,&render_pass_create_info, context->allocator, &out_renderpass->handle));

   DINFO("Renderpass created successfully");  
}

void vulkan_renderpass_destroy(vulkan_context*context, vulkan_renderpass* renderpass){
    if(renderpass && renderpass->handle){
           vkDestroyRenderPass(context->device.logical_device, renderpass->handle, context->allocator);
           renderpass->handle =0;
    }
}

void vulkan_renderpass_begin(
    vulkan_command_buffer*command_buffer,
    vulkan_renderpass* renderpass,
    VkFramebuffer frame_buffer
){
    VkRenderPassBeginInfo begin_info={VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
     begin_info.pNext = nullptr;
     begin_info.framebuffer = frame_buffer;
     begin_info.renderArea.offset.x = renderpass->x;
     begin_info.renderArea.offset.y = renderpass->y;
     begin_info.renderArea.extent.width = renderpass->w;
     begin_info.renderArea.extent.height = renderpass->h;
     begin_info.renderPass = renderpass->handle;
     u32 clear_value_count = 0;
     VkClearValue clear_values[2];
     Dzero_memory(clear_values, sizeof(VkClearValue)*2);
    //  bool do_clear_color = (renderpass->clear_flag & RENDERPASS_CLEAR_COLOR_BUFFER_FLAG) !=0;
     bool do_clear_depth = (renderpass->clear_flag & RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG) !=0;
    //  if(do_clear_color){
          clear_values[clear_value_count].color.float32[0] = renderpass->r;
          clear_values[clear_value_count].color.float32[1] = renderpass->g;
          clear_values[clear_value_count].color.float32[2] = renderpass->b;
          clear_values[clear_value_count].color.float32[3] = renderpass->a;
          clear_value_count++;
    //  }
     if(do_clear_depth){
          clear_values[clear_value_count].depthStencil.depth = renderpass->depth;
          bool do_clear_stencil = (renderpass->clear_flag & RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG) != 0;
          clear_values[clear_value_count].depthStencil.stencil= do_clear_stencil? renderpass->stencil: 0;
          clear_value_count++;
      }
     begin_info.clearValueCount =clear_value_count;
     begin_info.pClearValues = clear_value_count > 0 ? clear_values : nullptr;

     vkCmdBeginRenderPass(command_buffer->handle,&begin_info, VK_SUBPASS_CONTENTS_INLINE);
     command_buffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;            
};

void vulkan_renderpass_end(vulkan_command_buffer*command_buffer, vulkan_renderpass*renderpass){
  vkCmdEndRenderPass(command_buffer->handle);
  command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;   
}

