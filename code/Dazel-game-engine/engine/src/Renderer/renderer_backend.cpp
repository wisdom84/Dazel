#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"
#include "renderer_types.inl"
bool create_renderer_backend(u16 type, struct platform_state*plat_state, struct renderer_backend*out_renderer_backend){
    out_renderer_backend->plat_state = plat_state;
    if(type == RENDERER_BACKEND_TYPE_VULKAN){
       out_renderer_backend->initialize =vulkan_backend_initialize;
       out_renderer_backend->shutdown =vulkan_backend_shutdown;
       out_renderer_backend->resized = vulkan_backend_resized;
       out_renderer_backend->begin_frame =vulkan_backend_begin_frame;
       out_renderer_backend->end_frame=vulkan_backend_end_frame;
       out_renderer_backend->update_global_state =vulkan_renderer_update_global_state;
       out_renderer_backend->update_light_global_state =vulkan_renderer_update_light_global_state;
       out_renderer_backend->begin_renderpass = vulkan_renderer_begin_renderpass;
       out_renderer_backend->end_renderpass = vulkan_renderer_end_renderpass;
       out_renderer_backend->update_UI_global_state= vulkan_renderer_update_UI_global_state;
       out_renderer_backend->create_texture = vulkan_create_texture;
       out_renderer_backend->destroy_texture = vulkan_destroy_texture;
       out_renderer_backend->create_geometry = vulkan_create_geometry;
       out_renderer_backend->draw_geometry= vulkan_renderer_draw_geometry;
       out_renderer_backend->destroy_geometry = vulkan_destroy_geometry;
       out_renderer_backend->shader_create = vulkan_create_material_shader;
       out_renderer_backend->acquire_shader_resources = vulkan_acquire_material_shader_resources;
       out_renderer_backend->release_shader_resources = vulkan_release_material_shader_resources;
        return true;
    }
    return  false;
}
void renderer_backend_destroy( struct renderer_backend* renderer_backend){
    renderer_backend->initialize = 0;
    renderer_backend->shutdown =0;
    renderer_backend->begin_frame =0;
    renderer_backend->end_frame =0;
    renderer_backend->resized=0;
    renderer_backend->update_global_state =0;
    renderer_backend->create_texture = 0;
    renderer_backend->destroy_texture=0;
    renderer_backend->create_geometry = 0;
    renderer_backend->draw_geometry = 0;
    renderer_backend->destroy_geometry = 0;
};